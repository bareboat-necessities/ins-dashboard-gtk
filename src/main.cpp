#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cctype>
#include <cstring>
#include <deque>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr double PI = 3.1415926535897932384626433832795;
constexpr double DEG = PI / 180.0;

static double clampd(double x, double lo, double hi) {
    return std::max(lo, std::min(hi, x));
}

static double wrap360(double x) {
    while (x < 0.0) x += 360.0;
    while (x >= 360.0) x -= 360.0;
    return x;
}

static std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    return s;
}

static bool starts_with(const std::string& s, const std::string& p) {
    return s.rfind(p, 0) == 0;
}

static std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::string cur;
    std::istringstream iss(s);
    while (std::getline(iss, cur, sep)) out.push_back(cur);
    if (!s.empty() && s.back() == sep) out.emplace_back();
    return out;
}

static std::optional<double> to_double(const std::string& s) {
    try {
        size_t idx = 0;
        double v = std::stod(s, &idx);
        while (idx < s.size() && std::isspace(static_cast<unsigned char>(s[idx]))) idx++;
        if (idx != s.size()) return std::nullopt;
        return v;
    } catch (...) {
        return std::nullopt;
    }
}

static std::optional<int> to_int(const std::string& s) {
    try {
        size_t idx = 0;
        int v = std::stoi(s, &idx, 10);
        while (idx < s.size() && std::isspace(static_cast<unsigned char>(s[idx]))) idx++;
        if (idx != s.size()) return std::nullopt;
        return v;
    } catch (...) {
        return std::nullopt;
    }
}

static std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

struct InsData {
    double heading_deg = 127.0;
    double yaw_deg = 127.0;
    double roll_deg = -6.2;
    double pitch_deg = 2.1;
    double heave_m = 0.18;
    double heave_vel_mps = 0.11;
    double rot_deg_min = 8.4;
    double wave_rel_deg = 45.0;
    double wave_conf_pct = 78.0;
    double hs_m = 0.72;
    double tp_s = 4.8;
    double temp_c = 36.8;
    double sample_hz = 240.0;
    double mag_rate_hz = 100.0;

    std::string attitude_status = "GOOD";
    std::string heave_status = "GOOD";
    std::string wave_status = "FAIR";
    std::string mag_status = "LOCKED";
    std::string gyro_bias_status = "LEARNING";
    std::string acc_bias_status = "STABLE";
    std::string system_status = "INS GOOD";
    std::string last_sentence;

    std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();
    bool valid = false;
};

struct SharedModel {
    std::mutex mtx;
    InsData data;
    std::atomic<bool> stop{false};
};

struct SourceConfig {
    enum class Kind { Demo, Tcp, Serial } kind = Kind::Demo;
    std::string uri = "demo://";
    std::string host;
    int port = 0;
    std::string path;
    int baud = 115200;
};

static SourceConfig parse_source_uri(const std::string& uri) {
    SourceConfig c;
    c.uri = uri;

    if (uri == "demo://" || uri == "demo") {
        c.kind = SourceConfig::Kind::Demo;
        return c;
    }

    const std::string tcp_prefix = "tcp-nmea0183://";
    const std::string ser_prefix = "serial-nmea0183://";

    if (starts_with(uri, tcp_prefix)) {
        c.kind = SourceConfig::Kind::Tcp;
        std::string rest = uri.substr(tcp_prefix.size());
        if (rest.empty()) throw std::runtime_error("empty TCP source");

        if (rest.front() == '[') {
            auto close = rest.find(']');
            if (close == std::string::npos || close + 2 > rest.size() || rest[close + 1] != ':') {
                throw std::runtime_error("invalid IPv6 TCP source; expected tcp-nmea0183://[addr]:port");
            }
            c.host = rest.substr(1, close - 1);
            c.port = std::stoi(rest.substr(close + 2));
        } else {
            auto colon = rest.rfind(':');
            if (colon == std::string::npos) throw std::runtime_error("invalid TCP source; expected host:port");
            c.host = rest.substr(0, colon);
            c.port = std::stoi(rest.substr(colon + 1));
        }
        if (c.host.empty() || c.port <= 0 || c.port > 65535) throw std::runtime_error("invalid TCP source host/port");
        return c;
    }

    if (starts_with(uri, ser_prefix)) {
        c.kind = SourceConfig::Kind::Serial;
        std::string rest = uri.substr(ser_prefix.size());
        auto q = rest.find('?');
        c.path = q == std::string::npos ? rest : rest.substr(0, q);
        if (c.path.empty()) throw std::runtime_error("invalid serial source path");
        if (q != std::string::npos) {
            std::string query = rest.substr(q + 1);
            for (const auto& kv : split(query, '&')) {
                auto eq = kv.find('=');
                if (eq == std::string::npos) continue;
                std::string k = upper(kv.substr(0, eq));
                std::string v = kv.substr(eq + 1);
                if (k == "BAUD") {
                    auto b = to_int(v);
                    if (!b) throw std::runtime_error("invalid baud value: " + v);
                    c.baud = *b;
                }
            }
        }
        return c;
    }

    throw std::runtime_error("unsupported source URI: " + uri);
}

static speed_t baud_to_speed(int baud) {
    switch (baud) {
        case 50: return B50;
        case 75: return B75;
        case 110: return B110;
        case 134: return B134;
        case 150: return B150;
        case 200: return B200;
        case 300: return B300;
        case 600: return B600;
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
#ifdef B57600
        case 57600: return B57600;
#endif
#ifdef B115200
        case 115200: return B115200;
#endif
#ifdef B230400
        case 230400: return B230400;
#endif
#ifdef B460800
        case 460800: return B460800;
#endif
#ifdef B500000
        case 500000: return B500000;
#endif
#ifdef B576000
        case 576000: return B576000;
#endif
#ifdef B921600
        case 921600: return B921600;
#endif
#ifdef B1000000
        case 1000000: return B1000000;
#endif
#ifdef B1152000
        case 1152000: return B1152000;
#endif
#ifdef B1500000
        case 1500000: return B1500000;
#endif
#ifdef B2000000
        case 2000000: return B2000000;
#endif
#ifdef B3000000
        case 3000000: return B3000000;
#endif
#ifdef B4000000
        case 4000000: return B4000000;
#endif
        default:
            throw std::runtime_error("unsupported termios baud " + std::to_string(baud) +
                                     "; use a standard baud such as 9600, 38400, 115200, 230400, or 921600");
    }
}

static bool verify_nmea_checksum(const std::string& raw) {
    std::string s = trim(raw);
    auto star = s.find('*');
    if (star == std::string::npos) return true; // tolerate missing checksum for lab streams
    if (star + 2 >= s.size()) return false;

    size_t start = 0;
    if (!s.empty() && (s[0] == '$' || s[0] == '!')) start = 1;

    unsigned char x = 0;
    for (size_t i = start; i < star; ++i) x ^= static_cast<unsigned char>(s[i]);

    std::string hx = s.substr(star + 1, 2);
    unsigned int want = 0;
    std::istringstream iss(hx);
    iss >> std::hex >> want;
    return !iss.fail() && x == static_cast<unsigned char>(want & 0xff);
}

static std::string sentence_body_no_checksum(const std::string& raw) {
    std::string s = trim(raw);
    if (!s.empty() && (s[0] == '$' || s[0] == '!')) s.erase(s.begin());
    auto star = s.find('*');
    if (star != std::string::npos) s = s.substr(0, star);
    return s;
}

static void set_status_field(InsData& d, const std::string& key, const std::string& value) {
    std::string k = upper(key);
    std::string v = upper(value);
    if (k == "ATT" || k == "ATTITUDE") d.attitude_status = v;
    else if (k == "HEAVE") d.heave_status = v;
    else if (k == "WAVE" || k == "WAVEDIR" || k == "WAVE_DIR") d.wave_status = v;
    else if (k == "MAG") d.mag_status = v;
    else if (k == "GYRO" || k == "GYROBIAS" || k == "GYRO_BIAS") d.gyro_bias_status = v;
    else if (k == "ACC" || k == "ACCEL" || k == "ACCBIAS" || k == "ACC_BIAS") d.acc_bias_status = v;
    else if (k == "TEMP" || k == "TEMP_C") { if (auto x = to_double(value)) d.temp_c = *x; }
    else if (k == "SAMPLE" || k == "SAMPLE_HZ") { if (auto x = to_double(value)) d.sample_hz = *x; }
    else if (k == "MAGRATE" || k == "MAG_RATE" || k == "MAG_HZ") { if (auto x = to_double(value)) d.mag_rate_hz = *x; }
    else if (k == "SYSTEM" || k == "SYS") d.system_status = v;
}

static void parse_pins(const std::vector<std::string>& f, InsData& d) {
    if (f.size() < 2) return;

    std::string mode = upper(f[1]);
    if (mode == "ATT" || mode == "ATTITUDE") {
        if (f.size() > 2) if (auto x = to_double(f[2])) d.roll_deg = *x;
        if (f.size() > 3) if (auto x = to_double(f[3])) d.pitch_deg = *x;
        if (f.size() > 4) if (auto x = to_double(f[4])) d.heading_deg = d.yaw_deg = wrap360(*x);
        return;
    }
    if (mode == "HEAVE") {
        if (f.size() > 2) if (auto x = to_double(f[2])) d.heave_m = *x;
        if (f.size() > 3) if (auto x = to_double(f[3])) d.heave_vel_mps = *x;
        if (f.size() > 4) if (auto x = to_double(f[4])) d.hs_m = *x;
        if (f.size() > 5) if (auto x = to_double(f[5])) d.tp_s = *x;
        return;
    }
    if (mode == "WAVE" || mode == "WAVEDIR" || mode == "WAVE_DIR") {
        if (f.size() > 2) if (auto x = to_double(f[2])) d.wave_rel_deg = wrap360(*x);
        if (f.size() > 3) if (auto x = to_double(f[3])) d.wave_conf_pct = clampd(*x, 0.0, 100.0);
        return;
    }
    if (mode == "STATUS") {
        for (size_t i = 2; i < f.size(); ++i) {
            auto eq = f[i].find('=');
            if (eq == std::string::npos) continue;
            set_status_field(d, f[i].substr(0, eq), f[i].substr(eq + 1));
        }
        return;
    }

    // Compact numeric form:
    // $PINS,roll,pitch,yaw,heave,heaveVel,rotDegMin,waveRelDeg,waveConf,Hs,Tp,status
    if (f.size() >= 4) {
        if (auto x = to_double(f[1])) d.roll_deg = *x;
        if (auto x = to_double(f[2])) d.pitch_deg = *x;
        if (auto x = to_double(f[3])) d.heading_deg = d.yaw_deg = wrap360(*x);
    }
    if (f.size() >= 5) if (auto x = to_double(f[4])) d.heave_m = *x;
    if (f.size() >= 6) if (auto x = to_double(f[5])) d.heave_vel_mps = *x;
    if (f.size() >= 7) if (auto x = to_double(f[6])) d.rot_deg_min = *x;
    if (f.size() >= 8) if (auto x = to_double(f[7])) d.wave_rel_deg = wrap360(*x);
    if (f.size() >= 9) if (auto x = to_double(f[8])) d.wave_conf_pct = clampd(*x, 0.0, 100.0);
    if (f.size() >= 10) if (auto x = to_double(f[9])) d.hs_m = *x;
    if (f.size() >= 11) if (auto x = to_double(f[10])) d.tp_s = *x;
    if (f.size() >= 12) {
        std::string st = upper(f[11]);
        d.system_status = (st == "A" || st == "GOOD" || st == "OK") ? "INS GOOD" : "INS WARN";
    }
}

static void parse_xdr(const std::vector<std::string>& f, InsData& d) {
    // $--XDR,A,value,D,ROLL,A,value,D,PITCH,... groups of four after sentence id.
    for (size_t i = 1; i + 3 < f.size(); i += 4) {
        auto val = to_double(f[i + 1]);
        if (!val) continue;
        std::string name = upper(f[i + 3]);
        if (name.find("ROLL") != std::string::npos) d.roll_deg = *val;
        else if (name.find("PITCH") != std::string::npos) d.pitch_deg = *val;
        else if (name.find("YAW") != std::string::npos || name.find("HDG") != std::string::npos || name.find("HEAD") != std::string::npos) {
            d.heading_deg = d.yaw_deg = wrap360(*val);
        } else if (name.find("HEAVE") != std::string::npos) d.heave_m = *val;
        else if (name.find("ROT") != std::string::npos) d.rot_deg_min = *val;
        else if (name.find("WAVE") != std::string::npos) d.wave_rel_deg = wrap360(*val);
        else if (name == "HS") d.hs_m = *val;
        else if (name == "TP") d.tp_s = *val;
    }
}

static void parse_nmea_line(const std::string& raw, SharedModel& model) {
    std::string line = trim(raw);
    if (line.empty()) return;
    if ((line[0] != '$' && line[0] != '!') || !verify_nmea_checksum(line)) return;

    std::string body = sentence_body_no_checksum(line);
    auto f = split(body, ',');
    if (f.empty()) return;

    std::lock_guard<std::mutex> lock(model.mtx);
    InsData& d = model.data;
    d.last_sentence = line;
    d.valid = true;
    d.last_update = std::chrono::steady_clock::now();

    std::string id = upper(f[0]);
    std::string type = id.size() >= 3 ? id.substr(id.size() - 3) : id;

    if (id == "PINS") {
        parse_pins(f, d);
    } else if (id == "PRDID") {
        // Often: $PRDID,pitch,roll,heading
        if (f.size() > 1) if (auto x = to_double(f[1])) d.pitch_deg = *x;
        if (f.size() > 2) if (auto x = to_double(f[2])) d.roll_deg = *x;
        if (f.size() > 3) if (auto x = to_double(f[3])) d.heading_deg = d.yaw_deg = wrap360(*x);
    } else if (type == "HDT") {
        if (f.size() > 1) if (auto x = to_double(f[1])) d.heading_deg = d.yaw_deg = wrap360(*x);
    } else if (type == "HDG") {
        if (f.size() > 1) if (auto x = to_double(f[1])) d.heading_deg = d.yaw_deg = wrap360(*x);
    } else if (type == "ROT") {
        if (f.size() > 1) if (auto x = to_double(f[1])) d.rot_deg_min = *x;
        if (f.size() > 2 && upper(f[2]) != "A") d.system_status = "INS WARN";
    } else if (type == "XDR") {
        parse_xdr(f, d);
    }
}

static void update_demo(SharedModel& model, double t) {
    std::lock_guard<std::mutex> lock(model.mtx);
    auto& d = model.data;
    d.heading_deg = d.yaw_deg = wrap360(127.0 + 8.0 * std::sin(0.025 * t));
    d.roll_deg = -6.2 + 4.0 * std::sin(0.75 * t);
    d.pitch_deg = 2.1 + 2.0 * std::sin(0.53 * t + 1.1);
    d.heave_m = 0.24 * std::sin(1.35 * t) + 0.05 * std::sin(0.37 * t);
    d.heave_vel_mps = 0.24 * 1.35 * std::cos(1.35 * t) + 0.05 * 0.37 * std::cos(0.37 * t);
    d.rot_deg_min = 8.4 + 4.0 * std::sin(0.07 * t);
    d.wave_rel_deg = wrap360(45.0 + 10.0 * std::sin(0.05 * t));
    d.wave_conf_pct = 78.0 + 8.0 * std::sin(0.04 * t);
    d.hs_m = 0.72 + 0.08 * std::sin(0.03 * t);
    d.tp_s = 4.8 + 0.3 * std::sin(0.02 * t);
    d.temp_c = 36.8 + 0.2 * std::sin(0.01 * t);
    d.valid = true;
    d.last_update = std::chrono::steady_clock::now();
    d.last_sentence = "demo:// synthetic INS data";
}

class InputReader {
public:
    InputReader(SourceConfig cfg, SharedModel& model) : cfg_(std::move(cfg)), model_(model) {}
    void start() {
        th_ = std::thread([this] { run(); });
    }
    void join() {
        if (th_.joinable()) th_.join();
    }
private:
    SourceConfig cfg_;
    SharedModel& model_;
    std::thread th_;

    void run() {
        try {
            if (cfg_.kind == SourceConfig::Kind::Demo) run_demo();
            else if (cfg_.kind == SourceConfig::Kind::Tcp) run_tcp_forever();
            else if (cfg_.kind == SourceConfig::Kind::Serial) run_serial_forever();
        } catch (const std::exception& e) {
            std::cerr << "Input reader stopped: " << e.what() << "\n";
        }
    }

    void run_demo() {
        auto t0 = std::chrono::steady_clock::now();
        while (!model_.stop.load()) {
            auto now = std::chrono::steady_clock::now();
            double t = std::chrono::duration<double>(now - t0).count();
            update_demo(model_, t);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }

    void feed_bytes(const char* buf, ssize_t n, std::string& acc) {
        for (ssize_t i = 0; i < n; ++i) {
            char c = buf[i];
            if (c == '\n') {
                std::string line = trim(acc);
                acc.clear();
                if (!line.empty()) parse_nmea_line(line, model_);
            } else if (c != '\r') {
                if (acc.size() < 4096) acc.push_back(c);
                else acc.clear();
            }
        }
    }

    int connect_tcp_once() {
        struct addrinfo hints{};
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = AF_UNSPEC;
        std::string port_s = std::to_string(cfg_.port);
        struct addrinfo* res = nullptr;
        int rc = getaddrinfo(cfg_.host.c_str(), port_s.c_str(), &hints, &res);
        if (rc != 0) throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(rc));

        int fd = -1;
        for (auto* p = res; p; p = p->ai_next) {
            fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (fd < 0) continue;
            int yes = 1;
            setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));
            if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
            close(fd);
            fd = -1;
        }
        freeaddrinfo(res);
        if (fd < 0) throw std::runtime_error("TCP connect failed to " + cfg_.host + ":" + port_s + ": " + std::strerror(errno));
        return fd;
    }

    void run_tcp_forever() {
        std::string acc;
        while (!model_.stop.load()) {
            int fd = -1;
            try {
                fd = connect_tcp_once();
                std::cerr << "Connected to " << cfg_.uri << "\n";
                char buf[1024];
                while (!model_.stop.load()) {
                    pollfd pfd{fd, POLLIN, 0};
                    int pr = poll(&pfd, 1, 500);
                    if (pr < 0) {
                        if (errno == EINTR) continue;
                        throw std::runtime_error(std::string("poll: ") + std::strerror(errno));
                    }
                    if (pr == 0) continue;
                    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) break;
                    ssize_t n = read(fd, buf, sizeof(buf));
                    if (n > 0) feed_bytes(buf, n, acc);
                    else if (n == 0) break;
                    else if (errno != EINTR && errno != EAGAIN) throw std::runtime_error(std::string("read: ") + std::strerror(errno));
                }
            } catch (const std::exception& e) {
                std::cerr << "TCP input error: " << e.what() << "\n";
            }
            if (fd >= 0) close(fd);
            if (!model_.stop.load()) std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    int open_serial_once() {
        int fd = open(cfg_.path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (fd < 0) throw std::runtime_error("open serial " + cfg_.path + ": " + std::strerror(errno));

        termios tio{};
        if (tcgetattr(fd, &tio) != 0) {
            close(fd);
            throw std::runtime_error(std::string("tcgetattr: ") + std::strerror(errno));
        }

        cfmakeraw(&tio);
        tio.c_cflag |= CLOCAL | CREAD;
        tio.c_cflag &= ~CSIZE;
        tio.c_cflag |= CS8;
        tio.c_cflag &= ~PARENB;
        tio.c_cflag &= ~CSTOPB;
#ifdef CRTSCTS
        tio.c_cflag &= ~CRTSCTS;
#endif
        tio.c_cc[VMIN] = 0;
        tio.c_cc[VTIME] = 0;

        speed_t sp = baud_to_speed(cfg_.baud);
        if (cfsetispeed(&tio, sp) != 0 || cfsetospeed(&tio, sp) != 0) {
            close(fd);
            throw std::runtime_error(std::string("cfset speed: ") + std::strerror(errno));
        }
        if (tcsetattr(fd, TCSANOW, &tio) != 0) {
            close(fd);
            throw std::runtime_error(std::string("tcsetattr: ") + std::strerror(errno));
        }
        tcflush(fd, TCIOFLUSH);
        return fd;
    }

    void run_serial_forever() {
        std::string acc;
        while (!model_.stop.load()) {
            int fd = -1;
            try {
                fd = open_serial_once();
                std::cerr << "Opened " << cfg_.uri << "\n";
                char buf[512];
                while (!model_.stop.load()) {
                    pollfd pfd{fd, POLLIN, 0};
                    int pr = poll(&pfd, 1, 500);
                    if (pr < 0) {
                        if (errno == EINTR) continue;
                        throw std::runtime_error(std::string("serial poll: ") + std::strerror(errno));
                    }
                    if (pr == 0) continue;
                    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) break;
                    ssize_t n = read(fd, buf, sizeof(buf));
                    if (n > 0) feed_bytes(buf, n, acc);
                    else if (n < 0 && errno != EINTR && errno != EAGAIN) throw std::runtime_error(std::string("serial read: ") + std::strerror(errno));
                }
            } catch (const std::exception& e) {
                std::cerr << "Serial input error: " << e.what() << "\n";
            }
            if (fd >= 0) close(fd);
            if (!model_.stop.load()) std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};

struct Color { double r, g, b, a; };
static const Color BG{0.015, 0.025, 0.045, 1.0};
static const Color PANEL{0.02, 0.055, 0.095, 0.80};
static const Color PANEL2{0.025, 0.075, 0.13, 0.85};
static const Color LINE{0.16, 0.22, 0.34, 0.95};
static const Color WHITE{0.92, 0.94, 0.96, 1.0};
static const Color MUTED{0.55, 0.64, 0.74, 1.0};
static const Color CYAN{0.02, 0.76, 1.0, 1.0};
static const Color BLUE{0.02, 0.36, 0.85, 1.0};
static const Color GREEN{0.48, 0.94, 0.16, 1.0};
static const Color YELLOW{1.0, 0.78, 0.04, 1.0};
static const Color RED{1.0, 0.22, 0.16, 1.0};

static void setc(cairo_t* cr, Color c) { cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a); }

static void rounded_rect(cairo_t* cr, double x, double y, double w, double h, double r) {
    double rr = std::min(r, std::min(w, h) / 2.0);
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - rr, y + rr, rr, -PI / 2, 0);
    cairo_arc(cr, x + w - rr, y + h - rr, rr, 0, PI / 2);
    cairo_arc(cr, x + rr, y + h - rr, rr, PI / 2, PI);
    cairo_arc(cr, x + rr, y + rr, rr, PI, 3 * PI / 2);
    cairo_close_path(cr);
}

static void fill_round(cairo_t* cr, double x, double y, double w, double h, double r, Color fill, Color stroke = LINE, double lw = 2.0) {
    rounded_rect(cr, x, y, w, h, r);
    setc(cr, fill);
    cairo_fill_preserve(cr);
    setc(cr, stroke);
    cairo_set_line_width(cr, lw);
    cairo_stroke(cr);
}

static void line(cairo_t* cr, double x1, double y1, double x2, double y2, Color c, double lw=2.0) {
    setc(cr, c);
    cairo_set_line_width(cr, lw);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
}

static void text(cairo_t* cr, const std::string& s, double x, double y, double size, Color c,
                 const char* weight = "normal", double ax = 0.0, double ay = 0.0) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           std::string(weight) == "bold" ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, size);
    cairo_text_extents_t ext{};
    cairo_text_extents(cr, s.c_str(), &ext);
    double px = x - (ext.width * ax + ext.x_bearing);
    double py = y - (ext.height * ay + ext.y_bearing);
    setc(cr, c);
    cairo_move_to(cr, px, py);
    cairo_show_text(cr, s.c_str());
}

static std::string fmt(double v, int prec) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(prec) << v;
    return os.str();
}

static std::string fmt_signed(double v, int prec) {
    std::ostringstream os;
    os << (v >= 0 ? "+" : "") << std::fixed << std::setprecision(prec) << v;
    return os.str();
}

static std::string rel_wave_name(double deg) {
    deg = wrap360(deg);
    static const char* names[] = {"BOW", "STBD BOW", "STBD BEAM", "STBD QTR", "STERN", "PORT QTR", "PORT BEAM", "PORT BOW"};
    int idx = static_cast<int>(std::floor((deg + 22.5) / 45.0)) % 8;
    return names[idx];
}

static Color status_color(const std::string& s) {
    std::string u = upper(s);
    if (u.find("GOOD") != std::string::npos || u.find("LOCK") != std::string::npos || u.find("STABLE") != std::string::npos || u.find("HEALTH") != std::string::npos) return GREEN;
    if (u.find("FAIR") != std::string::npos || u.find("LEARN") != std::string::npos || u.find("WARN") != std::string::npos) return YELLOW;
    return RED;
}

static void draw_check_pill(cairo_t* cr, double x, double y, double w, double h, const std::string& s, Color c = GREEN) {
    fill_round(cr, x, y, w, h, h / 2.0, {0.05, 0.13, 0.05, 0.60}, c, 2.0);
    double cx = x + h * 0.50;
    double cy = y + h * 0.50;
    setc(cr, c);
    cairo_arc(cr, cx, cy, h * 0.25, 0, 2 * PI);
    cairo_fill(cr);
    line(cr, cx - h*0.11, cy, cx - h*0.03, cy + h*0.08, BG, 4);
    line(cr, cx - h*0.03, cy + h*0.08, cx + h*0.13, cy - h*0.11, BG, 4);
    text(cr, s, x + h * 0.90, y + h * 0.53, h * 0.34, c, "bold", 0.0, 0.5);
}

static void draw_wave_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 3.0 * scale);
    for (int k = 0; k < 3; ++k) {
        double yy = y + k * 12 * scale;
        cairo_move_to(cr, x, yy);
        for (int i = 0; i < 4; ++i) {
            cairo_rel_curve_to(cr, 7*scale, -8*scale, 15*scale, -8*scale, 22*scale, 0);
            cairo_rel_curve_to(cr, 7*scale, 8*scale, 15*scale, 8*scale, 22*scale, 0);
        }
        cairo_stroke(cr);
    }
}

static void draw_boat_top(cairo_t* cr, double cx, double cy, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 4 * scale);
    cairo_move_to(cr, cx, cy - 65*scale);
    cairo_curve_to(cr, cx - 36*scale, cy - 35*scale, cx - 32*scale, cy + 45*scale, cx - 22*scale, cy + 60*scale);
    cairo_line_to(cr, cx + 22*scale, cy + 60*scale);
    cairo_curve_to(cr, cx + 32*scale, cy + 45*scale, cx + 36*scale, cy - 35*scale, cx, cy - 65*scale);
    cairo_stroke(cr);
    rounded_rect(cr, cx - 20*scale, cy - 6*scale, 40*scale, 32*scale, 9*scale);
    cairo_stroke(cr);
}

static void draw_boat_front(cairo_t* cr, double cx, double cy, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 4 * scale);
    cairo_move_to(cr, cx - 50*scale, cy + 22*scale);
    cairo_line_to(cr, cx - 35*scale, cy - 26*scale);
    cairo_line_to(cr, cx + 35*scale, cy - 26*scale);
    cairo_line_to(cr, cx + 50*scale, cy + 22*scale);
    cairo_line_to(cr, cx + 26*scale, cy + 48*scale);
    cairo_line_to(cr, cx - 26*scale, cy + 48*scale);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    setc(cr, {0.94,0.96,0.98,0.9});
    cairo_fill(cr);
    setc(cr, BG);
    cairo_set_line_width(cr, 3*scale);
    cairo_move_to(cr, cx - 30*scale, cy + 12*scale);
    cairo_line_to(cr, cx + 30*scale, cy + 12*scale);
    cairo_stroke(cr);
    setc(cr, c);
    cairo_set_line_width(cr, 4*scale);
    cairo_move_to(cr, cx, cy - 44*scale);
    cairo_line_to(cr, cx, cy + 50*scale);
    cairo_stroke(cr);
}

struct AppState {
    SharedModel* model = nullptr;
    GtkWidget* drawing = nullptr;
    SourceConfig cfg;
    enum class Screen { Primary, Heave, Wave, Rot, Status } screen = Screen::Primary;
    std::deque<std::pair<double,double>> heave_hist;
    std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
};

static InsData snapshot(AppState* app) {
    std::lock_guard<std::mutex> lock(app->model->mtx);
    return app->model->data;
}

static void draw_title_bar(cairo_t* cr, const std::string& title, const std::string& right = "") {
    fill_round(cr, 20, 20, 960, 92, 18, {0.015,0.030,0.055,0.92}, LINE, 2);
    text(cr, title, 55, 66, 38, WHITE, "bold", 0, 0.5);
    if (!right.empty()) draw_check_pill(cr, 670, 38, 270, 54, right, GREEN);
}

static void draw_primary(cairo_t* cr, const InsData& d) {
    fill_round(cr, 20, 20, 960, 100, 18, {0.015,0.030,0.055,0.92}, LINE, 2);
    text(cr, "HDG", 60, 72, 40, WHITE, "bold", 0, 0.5);
    text(cr, fmt(d.heading_deg, 0) + "°", 185, 72, 72, CYAN, "bold", 0, 0.5);
    line(cr, 500, 28, 500, 112, LINE, 2);
    text(cr, "ROT", 555, 72, 40, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(d.rot_deg_min, 0) + "°", 685, 72, 72, CYAN, "bold", 0, 0.5);
    text(cr, "/min", 835, 82, 38, CYAN, "bold", 0, 0.5);

    double cx = 500, cy = 415, r = 300;
    setc(cr, LINE);
    cairo_set_line_width(cr, 5);
    cairo_arc(cr, cx, cy, r + 10, 0, 2*PI);
    cairo_stroke(cr);

    cairo_save(cr);
    cairo_arc(cr, cx, cy, r, 0, 2*PI);
    cairo_clip(cr);
    setc(cr, {0.02,0.35,0.78,1});
    cairo_paint(cr);
    cairo_save(cr);
    cairo_translate(cr, cx, cy);
    cairo_rotate(cr, d.roll_deg * DEG);
    double pitch_px = clampd(d.pitch_deg, -30, 30) * 5.0;
    setc(cr, {0.01,0.08,0.18,1});
    cairo_rectangle(cr, -600, pitch_px, 1200, 700);
    cairo_fill(cr);
    line(cr, -600, pitch_px, 600, pitch_px, WHITE, 4);
    setc(cr, WHITE);
    cairo_set_line_width(cr, 2);
    for (int p = -30; p <= 30; p += 10) {
        if (p == 0) continue;
        double y = pitch_px - p * 5.0;
        double len = (std::abs(p) % 20 == 0) ? 210 : 140;
        cairo_move_to(cr, -len/2, y);
        cairo_line_to(cr, len/2, y);
        cairo_stroke(cr);
        std::string ps = std::to_string(std::abs(p));
        text(cr, ps, -len/2 - 38, y, 24, WHITE, "bold", 0.5, 0.5);
        text(cr, ps, len/2 + 38, y, 24, WHITE, "bold", 0.5, 0.5);
    }
    cairo_restore(cr);
    draw_boat_front(cr, cx, cy + 15, 1.0, WHITE);
    cairo_restore(cr);

    // Roll tick ring.
    setc(cr, WHITE);
    cairo_set_line_width(cr, 3);
    for (int a = -60; a <= 60; a += 10) {
        double aa = (-90 + a) * DEG;
        double inner = r + 18;
        double outer = r + ((a % 30 == 0) ? 48 : 35);
        double x1 = cx + inner * std::cos(aa);
        double y1 = cy + inner * std::sin(aa);
        double x2 = cx + outer * std::cos(aa);
        double y2 = cy + outer * std::sin(aa);
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
        if (a % 30 == 0) {
            double tx = cx + (outer + 36) * std::cos(aa);
            double ty = cy + (outer + 36) * std::sin(aa);
            text(cr, std::to_string(std::abs(a)), tx, ty, 24, WHITE, "bold", 0.5, 0.5);
        }
    }
    setc(cr, CYAN);
    cairo_move_to(cr, cx, cy - r - 8);
    cairo_line_to(cr, cx - 16, cy - r - 42);
    cairo_line_to(cr, cx + 16, cy - r - 42);
    cairo_close_path(cr);
    cairo_fill(cr);

    fill_round(cr, 25, 708, 465, 106, 18, PANEL2, LINE, 2);
    fill_round(cr, 510, 708, 465, 106, 18, PANEL2, LINE, 2);
    text(cr, "ROLL", 180, 762, 32, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(d.roll_deg, 1) + "°", 300, 762, 56, CYAN, "bold", 0, 0.5);
    text(cr, "PITCH", 655, 762, 32, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(d.pitch_deg, 1) + "°", 795, 762, 56, CYAN, "bold", 0, 0.5);

    fill_round(cr, 25, 830, 950, 95, 18, PANEL2, LINE, 2);
    text(cr, "HEAVE", 180, 880, 30, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(d.heave_m, 2), 300, 882, 52, GREEN, "bold", 0, 0.5);
    text(cr, "m", 430, 886, 28, WHITE, "bold", 0, 0.5);
    text(cr, d.heave_vel_mps >= 0 ? "↑" : "↓", 500, 882, 64, GREEN, "bold", 0.5, 0.5);
    line(cr, 530, 845, 530, 910, LINE, 2);
    text(cr, "WAVES", 620, 862, 30, WHITE, "bold", 0, 0.5);
    text(cr, fmt(d.wave_rel_deg, 0) + "°", 635, 900, 48, CYAN, "bold", 0, 0.5);
    text(cr, rel_wave_name(d.wave_rel_deg), 780, 900, 28, WHITE, "bold", 0, 0.5);
    draw_check_pill(cr, 345, 940, 310, 45, d.system_status, status_color(d.system_status));
}

static void draw_heave(cairo_t* cr, const InsData& d, const std::deque<std::pair<double,double>>& hist, double now_s) {
    draw_title_bar(cr, "HEAVE", d.system_status);
    draw_wave_icon(cr, 80, 73, 0.7, CYAN);

    text(cr, fmt_signed(d.heave_m, 2), 330, 270, 120, CYAN, "bold", 0, 0.5);
    text(cr, "m", 785, 295, 50, CYAN, "bold", 0, 0.5);
    text(cr, d.heave_vel_mps >= 0 ? "↑" : "↓", 890, 285, 105, GREEN, "bold", 0.5, 0.5);
    text(cr, d.heave_vel_mps >= 0 ? "RISING" : "FALLING", 500, 372, 34, CYAN, "bold", 0.5, 0.5);

    fill_round(cr, 30, 430, 940, 82, 16, PANEL2, LINE, 2);
    text(cr, "VERTICAL SPEED", 120, 472, 31, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(d.heave_vel_mps, 2), 520, 472, 55, CYAN, "bold", 0, 0.5);
    text(cr, "m/s", 730, 476, 32, CYAN, "bold", 0, 0.5);

    double gx = 55, gy = 545, gw = 890, gh = 260;
    fill_round(cr, gx, gy, gw, gh, 18, {0.008,0.025,0.055,0.85}, LINE, 2);
    auto ymap = [&](double h) { return gy + gh/2.0 - clampd(h, -0.6, 0.6) / 0.6 * (gh/2.0 - 20); };
    line(cr, gx+80, ymap(0), gx+gw-25, ymap(0), MUTED, 1.5);
    text(cr, "+0.5 m", gx+22, ymap(0.5), 22, WHITE, "normal", 0, 0.5);
    text(cr, "0", gx+72, ymap(0), 22, WHITE, "normal", 1, 0.5);
    text(cr, "-0.5 m", gx+22, ymap(-0.5), 22, WHITE, "normal", 0, 0.5);
    for (int i = 0; i <= 4; ++i) {
        double x = gx + 80 + i * (gw - 120) / 4.0;
        line(cr, x, gy + 20, x, gy + gh - 50, {0.25,0.35,0.48,0.50}, 1);
    }
    setc(cr, {0.02,0.60,1.0,0.22});
    cairo_move_to(cr, gx+80, ymap(0));
    bool have = false;
    for (auto [t, h] : hist) {
        double age = now_s - t;
        if (age < 0 || age > 20) continue;
        double x = gx + 80 + (20.0 - age) / 20.0 * (gw - 120);
        double y = ymap(h);
        if (!have) { cairo_move_to(cr, x, y); have = true; }
        else cairo_line_to(cr, x, y);
    }
    if (have) {
        cairo_line_to(cr, gx+gw-40, ymap(0));
        cairo_line_to(cr, gx+80, ymap(0));
        cairo_close_path(cr);
        cairo_fill(cr);
    }
    setc(cr, CYAN);
    cairo_set_line_width(cr, 3.5);
    have = false;
    for (auto [t, h] : hist) {
        double age = now_s - t;
        if (age < 0 || age > 20) continue;
        double x = gx + 80 + (20.0 - age) / 20.0 * (gw - 120);
        double y = ymap(h);
        if (!have) { cairo_move_to(cr, x, y); have = true; }
        else cairo_line_to(cr, x, y);
    }
    if (have) cairo_stroke(cr);
    text(cr, "LAST 20 SECONDS", 500, gy + gh - 18, 24, CYAN, "bold", 0.5, 0.5);

    fill_round(cr, 30, 835, 455, 105, 18, PANEL2, LINE, 2);
    fill_round(cr, 515, 835, 455, 105, 18, PANEL2, LINE, 2);
    text(cr, "Hs", 180, 890, 32, WHITE, "bold", 0, 0.5);
    text(cr, fmt(d.hs_m, 2), 250, 892, 58, CYAN, "bold", 0, 0.5);
    text(cr, "m", 415, 898, 30, CYAN, "bold", 0, 0.5);
    text(cr, "Tp", 660, 890, 32, WHITE, "bold", 0, 0.5);
    text(cr, fmt(d.tp_s, 1), 750, 892, 58, CYAN, "bold", 0, 0.5);
    text(cr, "s", 895, 898, 30, CYAN, "bold", 0, 0.5);
}

static void draw_wave(cairo_t* cr, const InsData& d) {
    fill_round(cr, 20, 20, 960, 105, 18, {0.015,0.030,0.055,0.92}, LINE, 2);
    text(cr, "WAVE DIRECTION", 50, 55, 36, WHITE, "bold", 0, 0.5);
    text(cr, "RELATIVE TO VESSEL HEADING", 50, 93, 24, CYAN, "bold", 0, 0.5);
    draw_check_pill(cr, 690, 43, 250, 55, "CONF " + fmt(d.wave_conf_pct, 0) + "%", GREEN);

    double cx = 500, cy = 480, r = 300;
    setc(cr, LINE);
    cairo_set_line_width(cr, 3);
    cairo_arc(cr, cx, cy, r, 0, 2*PI);
    cairo_stroke(cr);
    for (int rr = 100; rr <= 220; rr += 60) {
        setc(cr, {0.02,0.45,1.0,0.35});
        cairo_set_line_width(cr, 1.5);
        cairo_set_dash(cr, nullptr, 0, 0);
        cairo_arc(cr, cx, cy, rr, 0, 2*PI);
        cairo_stroke(cr);
    }
    line(cr, cx-r, cy, cx+r, cy, {0.02,0.45,1,0.45}, 1.5);
    line(cr, cx, cy-r, cx, cy+r, {0.02,0.45,1,0.45}, 1.5);

    for (int a = 0; a < 360; a += 5) {
        double theta = (a - 90) * DEG;
        double inner = r - ((a % 30 == 0) ? 18 : 10);
        double outer = r;
        line(cr, cx + inner*std::cos(theta), cy + inner*std::sin(theta),
                 cx + outer*std::cos(theta), cy + outer*std::sin(theta), CYAN, a%30==0 ? 2.5 : 1.2);
        if (a % 30 == 0) {
            double tx = cx + (r + 36) * std::cos(theta);
            double ty = cy + (r + 36) * std::sin(theta);
            text(cr, std::to_string(a) + "°", tx, ty, 23, WHITE, "normal", 0.5, 0.5);
        }
    }
    text(cr, "BOW", cx, cy-r-55, 28, WHITE, "bold", 0.5, 0.5);
    text(cr, "STERN", cx, cy+r+55, 28, WHITE, "bold", 0.5, 0.5);
    text(cr, "PORT", cx-r-95, cy, 28, WHITE, "bold", 0.5, 0.5);
    text(cr, "STBD", cx+r+95, cy, 28, WHITE, "bold", 0.5, 0.5);
    draw_boat_top(cr, cx, cy, 0.95, WHITE);

    // Wave arrow: waves coming FROM rel angle toward vessel. 0 deg = bow, clockwise positive.
    double theta = (d.wave_rel_deg - 90.0) * DEG;
    double sx = cx + 210 * std::cos(theta);
    double sy = cy + 210 * std::sin(theta);
    double ex = cx + 85 * std::cos(theta);
    double ey = cy + 85 * std::sin(theta);
    line(cr, sx, sy, ex, ey, CYAN, 10);
    double ah = 22;
    double dir = std::atan2(ey - sy, ex - sx);
    setc(cr, CYAN);
    cairo_move_to(cr, ex, ey);
    cairo_line_to(cr, ex - ah * std::cos(dir - 0.45), ey - ah * std::sin(dir - 0.45));
    cairo_line_to(cr, ex - ah * std::cos(dir + 0.45), ey - ah * std::sin(dir + 0.45));
    cairo_close_path(cr);
    cairo_fill(cr);
    draw_wave_icon(cr, sx - 35, sy - 35, 0.55, CYAN);

    fill_round(cr, 20, 825, 960, 140, 18, PANEL2, LINE, 2);
    text(cr, fmt(d.wave_rel_deg, 0) + "°", 70, 895, 82, CYAN, "bold", 0, 0.5);
    text(cr, "REL", 335, 912, 34, CYAN, "bold", 0, 0.5);
    line(cr, 500, 845, 500, 945, LINE, 2);
    draw_wave_icon(cr, 535, 875, 0.55, CYAN);
    text(cr, "FROM", 650, 872, 32, WHITE, "bold", 0, 0.5);
    text(cr, rel_wave_name(d.wave_rel_deg), 650, 922, 50, CYAN, "bold", 0, 0.5);
}

static void draw_rot(cairo_t* cr, const InsData& d) {
    draw_title_bar(cr, "RATE OF TURN");
    text(cr, "PORT", 95, 215, 40, RED, "bold", 0, 0.5);
    text(cr, "STBD", 810, 215, 40, GREEN, "bold", 0, 0.5);
    double cx = 500, cy = 560, r = 365;
    setc(cr, {0.02,0.17,0.45,0.85});
    cairo_arc(cr, cx, cy, r, PI, 2*PI);
    cairo_line_to(cr, cx, cy);
    cairo_close_path(cr);
    cairo_fill_preserve(cr);
    setc(cr, LINE);
    cairo_set_line_width(cr, 4);
    cairo_stroke(cr);

    for (int v = -60; v <= 60; v += 5) {
        double angle = (-90 + v * 1.5) * DEG;
        double inner = r - ((v % 30 == 0) ? 52 : (v % 10 == 0 ? 38 : 25));
        double outer = r;
        Color cc = WHITE;
        if (v <= -55) cc = RED;
        if (v >= 55) cc = GREEN;
        line(cr, cx + inner*std::cos(angle), cy + inner*std::sin(angle),
                 cx + outer*std::cos(angle), cy + outer*std::sin(angle), cc, v%10==0 ? 3.0 : 1.5);
        if (v % 30 == 0) {
            double tx = cx + (inner - 40) * std::cos(angle);
            double ty = cy + (inner - 40) * std::sin(angle);
            text(cr, std::to_string(std::abs(v)), tx, ty, 28, WHITE, "bold", 0.5, 0.5);
        }
    }
    text(cr, "°/min", cx, cy - 235, 34, CYAN, "bold", 0.5, 0.5);
    double val = clampd(d.rot_deg_min, -60, 60);
    double angle = (-90 + val * 1.5) * DEG;
    double nx = cx + (r - 100) * std::cos(angle);
    double ny = cy + (r - 100) * std::sin(angle);
    line(cr, cx, cy, nx, ny, WHITE, 10);
    setc(cr, {0.08,0.12,0.18,1});
    cairo_arc(cr, cx, cy, 42, 0, 2*PI);
    cairo_fill_preserve(cr);
    setc(cr, LINE);
    cairo_set_line_width(cr, 2);
    cairo_stroke(cr);

    fill_round(cr, 50, 610, 900, 135, 18, PANEL2, LINE, 2);
    text(cr, fmt_signed(d.rot_deg_min, 1) + "°", 150, 680, 82, CYAN, "bold", 0, 0.5);
    text(cr, "/min", 485, 695, 44, CYAN, "bold", 0, 0.5);
    line(cr, 625, 635, 625, 720, LINE, 2);
    text(cr, "TURNING", 700, 656, 35, WHITE, "bold", 0, 0.5);
    text(cr, d.rot_deg_min >= 0 ? "STBD" : "PORT", 700, 705, 55, d.rot_deg_min >= 0 ? GREEN : RED, "bold", 0, 0.5);

    fill_round(cr, 50, 765, 900, 105, 18, PANEL2, LINE, 2);
    text(cr, "HDG TREND", 70, 815, 28, WHITE, "bold", 0, 0.5);
    double h1 = wrap360(d.heading_deg - d.rot_deg_min / 6.0);
    double h3 = wrap360(d.heading_deg + d.rot_deg_min / 6.0);
    text(cr, fmt(h1,0)+"°", 300, 805, 42, CYAN, "bold", 0.5, 0.5);
    text(cr, "-10 sec", 300, 850, 25, CYAN, "bold", 0.5, 0.5);
    text(cr, "→", 430, 805, 40, WHITE, "bold", 0.5, 0.5);
    text(cr, fmt(d.heading_deg,0)+"°", 560, 805, 42, CYAN, "bold", 0.5, 0.5);
    text(cr, "NOW", 560, 850, 25, CYAN, "bold", 0.5, 0.5);
    text(cr, "→", 690, 805, 40, WHITE, "bold", 0.5, 0.5);
    text(cr, fmt(h3,0)+"°", 820, 805, 42, CYAN, "bold", 0.5, 0.5);
    text(cr, "+10 sec", 820, 850, 25, CYAN, "bold", 0.5, 0.5);
    draw_check_pill(cr, 345, 915, 310, 50, d.system_status, status_color(d.system_status));
}

static void draw_status_row(cairo_t* cr, int idx, const std::string& label, const std::string& value, Color vc) {
    double y = 140 + idx * 82;
    fill_round(cr, 38, y, 924, 65, 12, PANEL2, LINE, 1.5);
    text(cr, label, 205, y + 34, 31, WHITE, "bold", 0, 0.5);
    line(cr, 520, y + 12, 520, y + 53, LINE, 1.5);
    setc(cr, vc);
    cairo_arc(cr, 575, y + 34, 12, 0, 2*PI);
    cairo_fill(cr);
    text(cr, value, 625, y + 34, 34, vc, "bold", 0, 0.5);
}

static void draw_status(cairo_t* cr, const InsData& d) {
    fill_round(cr, 20, 20, 960, 945, 18, {0.012,0.025,0.045,0.92}, LINE, 2);
    text(cr, "INS STATUS", 60, 75, 55, WHITE, "bold", 0, 0.5);
    draw_check_pill(cr, 590, 42, 350, 60, "SYSTEM HEALTHY", GREEN);
    draw_status_row(cr, 0, "ATTITUDE", d.attitude_status, status_color(d.attitude_status));
    draw_status_row(cr, 1, "HEAVE", d.heave_status, status_color(d.heave_status));
    draw_status_row(cr, 2, "WAVE DIR", d.wave_status, status_color(d.wave_status));
    draw_status_row(cr, 3, "MAG", d.mag_status, status_color(d.mag_status));
    draw_status_row(cr, 4, "GYRO BIAS", d.gyro_bias_status, status_color(d.gyro_bias_status));
    draw_status_row(cr, 5, "ACC BIAS", d.acc_bias_status, status_color(d.acc_bias_status));
    draw_status_row(cr, 6, "TEMP", fmt(d.temp_c,1) + " °C", CYAN);
    draw_status_row(cr, 7, "SAMPLE", fmt(d.sample_hz,0) + " Hz", CYAN);
    draw_status_row(cr, 8, "MAG RATE", fmt(d.mag_rate_hz,0) + " Hz", CYAN);
}

static void draw_cb(GtkDrawingArea*, cairo_t* cr, int width, int height, gpointer user_data) {
    auto* app = static_cast<AppState*>(user_data);
    InsData d = snapshot(app);

    double side = std::min(width, height);
    double ox = (width - side) * 0.5;
    double oy = (height - side) * 0.5;
    cairo_save(cr);
    setc(cr, BG);
    cairo_paint(cr);
    cairo_translate(cr, ox, oy);
    cairo_scale(cr, side / 1000.0, side / 1000.0);

    fill_round(cr, 10, 10, 980, 980, 18, {0.006,0.014,0.026,1.0}, LINE, 2.0);

    auto now = std::chrono::steady_clock::now();
    double now_s = std::chrono::duration<double>(now - app->t0).count();

    switch (app->screen) {
        case AppState::Screen::Primary: draw_primary(cr, d); break;
        case AppState::Screen::Heave: draw_heave(cr, d, app->heave_hist, now_s); break;
        case AppState::Screen::Wave: draw_wave(cr, d); break;
        case AppState::Screen::Rot: draw_rot(cr, d); break;
        case AppState::Screen::Status: draw_status(cr, d); break;
    }

    text(cr, "1 HELM   2 HEAVE   3 WAVES   4 ROT   5 STATUS", 500, 984, 14, MUTED, "bold", 0.5, 0.5);
    cairo_restore(cr);
}

static gboolean tick_cb(gpointer user_data) {
    auto* app = static_cast<AppState*>(user_data);
    InsData d = snapshot(app);
    auto now = std::chrono::steady_clock::now();
    double t = std::chrono::duration<double>(now - app->t0).count();
    app->heave_hist.emplace_back(t, d.heave_m);
    while (!app->heave_hist.empty() && t - app->heave_hist.front().first > 20.5) app->heave_hist.pop_front();
    gtk_widget_queue_draw(app->drawing);
    return G_SOURCE_CONTINUE;
}

static gboolean key_cb(GtkEventControllerKey*, guint keyval, guint, GdkModifierType, gpointer user_data) {
    auto* app = static_cast<AppState*>(user_data);
    switch (keyval) {
        case GDK_KEY_1: app->screen = AppState::Screen::Primary; break;
        case GDK_KEY_2: app->screen = AppState::Screen::Heave; break;
        case GDK_KEY_3: app->screen = AppState::Screen::Wave; break;
        case GDK_KEY_4: app->screen = AppState::Screen::Rot; break;
        case GDK_KEY_5: app->screen = AppState::Screen::Status; break;
        case GDK_KEY_Right:
        case GDK_KEY_space: {
            int n = static_cast<int>(app->screen);
            app->screen = static_cast<AppState::Screen>((n + 1) % 5);
            break;
        }
        case GDK_KEY_Left: {
            int n = static_cast<int>(app->screen);
            app->screen = static_cast<AppState::Screen>((n + 4) % 5);
            break;
        }
        case GDK_KEY_Escape:
        case GDK_KEY_q:
        case GDK_KEY_Q: {
            GtkRoot* root = gtk_widget_get_root(app->drawing);
            if (GTK_IS_WINDOW(root)) gtk_window_close(GTK_WINDOW(root));
            return TRUE;
        }
        default:
            return FALSE;
    }
    gtk_widget_queue_draw(app->drawing);
    return TRUE;
}

static void activate_cb(GtkApplication* gtk_app, gpointer user_data) {
    auto* app = static_cast<AppState*>(user_data);

    GtkWidget* win = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(win), "INS GTK4 NMEA Display");
    gtk_window_set_default_size(GTK_WINDOW(win), 900, 900);

    GtkWidget* area = gtk_drawing_area_new();
    app->drawing = area;
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(area), 720);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(area), 720);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), draw_cb, app, nullptr);
    gtk_window_set_child(GTK_WINDOW(win), area);

    GtkEventController* key = gtk_event_controller_key_new();
    g_signal_connect(key, "key-pressed", G_CALLBACK(key_cb), app);
    gtk_widget_add_controller(win, key);

    g_timeout_add(33, tick_cb, app);
    gtk_window_present(GTK_WINDOW(win));
}

static void print_usage(const char* argv0) {
    std::cerr << "Usage:\n"
              << "  " << argv0 << " --source tcp-nmea0183://host:port\n"
              << "  " << argv0 << " --source serial-nmea0183:///dev/ttyUSB0?baud=115200\n"
              << "  " << argv0 << " --source demo://\n";
}

} // namespace

int main(int argc, char** argv) {
    std::string source = "demo://";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--source" && i + 1 < argc) {
            source = argv[++i];
        } else if (a == "--help" || a == "-h") {
            print_usage(argv[0]);
            return 0;
        }
    }

    SourceConfig cfg;
    try {
        cfg = parse_source_uri(source);
    } catch (const std::exception& e) {
        std::cerr << "Source URI error: " << e.what() << "\n";
        print_usage(argv[0]);
        return 2;
    }

    SharedModel model;
    AppState app_state;
    app_state.model = &model;
    app_state.cfg = cfg;

    InputReader reader(cfg, model);
    reader.start();

    GtkApplication* app = gtk_application_new("com.example.InsGtk4Nmea", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate_cb), &app_state);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    model.stop.store(true);
    reader.join();
    return status;
}
