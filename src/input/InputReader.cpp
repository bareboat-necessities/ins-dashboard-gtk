#include "input/InputReader.h"

#include "nmea/NmeaParser.h"
#include "sim/DemoData.h"
#include "util/StringUtil.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <chrono>
#include <thread>
#include <utility>

namespace ins_display::input {
namespace {

speed_t baud_to_speed(int baud) {
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

} // namespace

InputReader::InputReader(SourceConfig cfg, model::SharedModel& model)
    : cfg_(std::move(cfg)), model_(model) {}

InputReader::~InputReader() {
    join();
}

void InputReader::start() {
    th_ = std::thread([this] { run(); });
}

void InputReader::join() {
    if (th_.joinable()) th_.join();
}

void InputReader::run() {
    try {
        if (cfg_.kind == SourceConfig::Kind::Demo) run_demo();
        else if (cfg_.kind == SourceConfig::Kind::Tcp) run_tcp_forever();
        else if (cfg_.kind == SourceConfig::Kind::Serial) run_serial_forever();
    } catch (const std::exception& e) {
        std::cerr << "Input reader stopped: " << e.what() << "\n";
    }
}

void InputReader::run_demo() {
    auto t0 = std::chrono::steady_clock::now();
    while (!model_.stop.load()) {
        auto now = std::chrono::steady_clock::now();
        double t = std::chrono::duration<double>(now - t0).count();
        sim::update_demo(model_, t);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

void InputReader::feed_bytes(const char* buf, ssize_t n, std::string& acc) {
    for (ssize_t i = 0; i < n; ++i) {
        char c = buf[i];
        if (c == '\n') {
            std::string line = util::trim(acc);
            acc.clear();
            if (!line.empty()) nmea::parse_nmea_line(line, model_);
        } else if (c != '\r') {
            if (acc.size() < 4096) acc.push_back(c);
            else acc.clear();
        }
    }
}

int InputReader::connect_tcp_once() {
    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    const std::string port_s = std::to_string(cfg_.port);
    addrinfo* res = nullptr;
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

    if (fd < 0) {
        throw std::runtime_error("TCP connect failed to " + cfg_.host + ":" + port_s + ": " + std::strerror(errno));
    }
    return fd;
}

void InputReader::run_tcp_forever() {
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

int InputReader::open_serial_once() {
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

void InputReader::run_serial_forever() {
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

} // namespace ins_display::input
