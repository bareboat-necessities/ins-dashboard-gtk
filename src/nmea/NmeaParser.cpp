#include "nmea/NmeaParser.h"

#include "util/MathUtil.h"
#include "util/StringUtil.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include <mutex>

namespace ins_display::nmea {
namespace {
using model::InsData;
using util::clampd;
using util::split;
using util::to_double;
using util::trim;
using util::upper;
using util::wrap360;

void set_status_field(InsData& d, const std::string& key, const std::string& value) {
    const std::string k = upper(key);
    const std::string v = upper(value);

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

void parse_pins(const std::vector<std::string>& f, InsData& d) {
    if (f.size() < 2) return;

    const std::string mode = upper(f[1]);
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

void parse_xdr(const std::vector<std::string>& f, InsData& d) {
    // $--XDR,A,value,D,ROLL,A,value,D,PITCH,... groups of four after sentence id.
    for (size_t i = 1; i + 3 < f.size(); i += 4) {
        auto val = to_double(f[i + 1]);
        if (!val) continue;
        const std::string name = upper(f[i + 3]);
        if (name.find("ROLL") != std::string::npos) d.roll_deg = *val;
        else if (name.find("PITCH") != std::string::npos) d.pitch_deg = *val;
        else if (name.find("YAW") != std::string::npos || name.find("HDG") != std::string::npos || name.find("HEAD") != std::string::npos) d.heading_deg = d.yaw_deg = wrap360(*val);
        else if (name.find("HEAVE") != std::string::npos) d.heave_m = *val;
        else if (name.find("ROT") != std::string::npos) d.rot_deg_min = *val;
        else if (name.find("WAVE") != std::string::npos) d.wave_rel_deg = wrap360(*val);
        else if (name == "HS") d.hs_m = *val;
        else if (name == "TP") d.tp_s = *val;
    }
}

} // namespace

bool verify_nmea_checksum(const std::string& raw) {
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

std::string sentence_body_no_checksum(const std::string& raw) {
    std::string s = trim(raw);
    if (!s.empty() && (s[0] == '$' || s[0] == '!')) s.erase(s.begin());
    auto star = s.find('*');
    if (star != std::string::npos) s = s.substr(0, star);
    return s;
}

void parse_nmea_line(const std::string& raw, model::SharedModel& model) {
    std::string line = trim(raw);
    if (line.empty()) return;
    if ((line[0] != '$' && line[0] != '!') || !verify_nmea_checksum(line)) return;

    const std::string body = sentence_body_no_checksum(line);
    const auto f = split(body, ',');
    if (f.empty()) return;

    std::lock_guard<std::mutex> lock(model.mtx);
    InsData& d = model.data;
    d.last_sentence = line;
    d.valid = true;
    d.last_update = std::chrono::steady_clock::now();

    const std::string id = upper(f[0]);
    const std::string type = id.size() >= 3 ? id.substr(id.size() - 3) : id;

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

} // namespace ins_display::nmea
