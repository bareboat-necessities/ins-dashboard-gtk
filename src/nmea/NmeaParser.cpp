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

int direction_flag_to_sign(const std::string& raw) {
    const std::string v = upper(trim(raw));
    if (v.empty()) return 1;

    if (v == "-" || v == "NEG" || v == "NEGATIVE" || v == "PORT" || v == "P" ||
        v == "LEFT" || v == "L" || v == "CCW") {
        return -1;
    }
    if (v == "+" || v == "POS" || v == "POSITIVE" || v == "STBD" || v == "STARBOARD" ||
        v == "S" || v == "RIGHT" || v == "R" || v == "CW") {
        return 1;
    }

    if (auto n = util::to_int(v)) {
        if (*n < 0) return -1;
        if (*n > 0) return 1;
        return -1;
    }
    if (auto x = util::to_double(v)) {
        if (*x < 0.0) return -1;
        if (*x > 0.0) return 1;
        return -1;
    }

    return 1;
}

double signed_wave_degrees(const InsData& d, double magnitude_deg) {
    return static_cast<double>(d.wave_sign * d.wave_polarity) * std::abs(magnitude_deg);
}

void apply_wave_degrees(InsData& d, double value_deg) {
    d.wave_rel_mag_deg = std::abs(value_deg);
    d.wave_rel_deg = value_deg < 0.0 ? wrap360(value_deg) : wrap360(signed_wave_degrees(d, d.wave_rel_mag_deg));
    d.wave_status = "GOOD";
}

void reapply_wave_sign(InsData& d) {
    d.wave_rel_deg = wrap360(signed_wave_degrees(d, d.wave_rel_mag_deg));
}

std::string health_status_from_flag(const std::string& raw, const std::string& good = "GOOD", const std::string& bad = "WARN") {
    const std::string v = upper(trim(raw));
    if (v == "Y" || v == "YES" || v == "1" || v == "TRUE" || v == "A" || v == "OK" || v == "GOOD" ||
        v == "LOCK" || v == "LOCKED" || v == "ON") {
        return good;
    }
    if (v == "N" || v == "NO" || v == "0" || v == "FALSE" || v == "V" || v == "BAD" || v == "WARN" ||
        v == "OFF") {
        return bad;
    }
    return upper(raw);
}

std::string activity_status_from_flag(const std::string& raw, const std::string& active, const std::string& inactive) {
    const std::string v = upper(trim(raw));
    if (v == "Y" || v == "YES" || v == "1" || v == "TRUE" || v == "A" || v == "ON" || v == active) {
        return active;
    }
    if (v == "N" || v == "NO" || v == "0" || v == "FALSE" || v == "V" || v == "OFF" || v == inactive) {
        return inactive;
    }
    return upper(raw);
}

bool status_is_good(const std::string& raw) {
    const std::string v = upper(trim(raw));
    if (v.find("UNLOCK") != std::string::npos || v.find("WARN") != std::string::npos ||
        v.find("BAD") != std::string::npos || v == "OFF" || v == "N") {
        return false;
    }
    return v.find("GOOD") != std::string::npos || v.find("LOCK") != std::string::npos ||
           v.find("STABLE") != std::string::npos || v == "OK" || v == "ON" || v == "Y";
}

void refresh_system_status(InsData& d) {
    d.system_status = (status_is_good(d.attitude_status) && status_is_good(d.heave_status) &&
                       status_is_good(d.mag_status))
                          ? "INS GOOD"
                          : "INS WARN";
}

void set_status_field(InsData& d, const std::string& key, const std::string& value) {
    const std::string k = upper(key);

    if (k == "ATT" || k == "ATTITUDE") {
        d.attitude_status = health_status_from_flag(value);
        refresh_system_status(d);
    } else if (k == "HEV" || k == "HEAVE") {
        d.heave_status = health_status_from_flag(value);
        refresh_system_status(d);
    } else if (k == "WAVE" || k == "WAVEDIR" || k == "WAVE_DIR") {
        d.wave_status = health_status_from_flag(value);
    } else if (k == "MAG") {
        d.mag_status = health_status_from_flag(value, "LOCKED", "UNLOCKED");
        refresh_system_status(d);
    } else if (k == "GB" || k == "GYRO" || k == "GYROBIAS" || k == "GYRO_BIAS") {
        d.gyro_bias_status = activity_status_from_flag(value, "LEARNING", "STABLE");
    } else if (k == "AB" || k == "ACC" || k == "ACCEL" || k == "ACCBIAS" || k == "ACC_BIAS") {
        d.acc_bias_status = activity_status_from_flag(value, "ESTIMATING", "STABLE");
    } else if (k == "TEMP" || k == "TEMP_C" || k == "IMUT") {
        if (auto x = to_double(value)) d.temp_c = *x;
    } else if (k == "SAMPLE" || k == "SAMPLE_HZ" || k == "IHZ" || k == "IMUHZ" || k == "IMU_HZ") {
        if (auto x = to_double(value)) d.sample_hz = *x;
    } else if (k == "MAGRATE" || k == "MAG_RATE" || k == "MAG_HZ" || k == "MHZ" || k == "MAGHZ") {
        if (auto x = to_double(value)) d.mag_rate_hz = *x;
    } else if (k == "WAVCONF" || k == "WAVE_CONF" || k == "WAVEDIRCONF" || k == "WAVE_DIR_CONF") {
        if (auto x = to_double(value)) d.wave_conf_pct = clampd(*x, 0.0, 100.0);
    } else if (k == "SYSTEM" || k == "SYS") {
        d.system_status = upper(value);
    }
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
        if (f.size() > 3) if (auto x = to_double(f[3])) { d.heave_vel_mps = *x; d.heave_vel_available = true; }
        if (f.size() > 4) if (auto x = to_double(f[4])) d.hs_m = *x;
        if (f.size() > 5) if (auto x = to_double(f[5])) d.tp_s = *x;
        return;
    }

    if (mode == "WAVE" || mode == "WAVEDIR" || mode == "WAVE_DIR") {
        if (f.size() > 2) if (auto x = to_double(f[2])) apply_wave_degrees(d, *x);
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
    if (f.size() >= 6) if (auto x = to_double(f[5])) { d.heave_vel_mps = *x; d.heave_vel_available = true; }
    if (f.size() >= 7) if (auto x = to_double(f[6])) d.rot_deg_min = *x;
    if (f.size() >= 8) if (auto x = to_double(f[7])) apply_wave_degrees(d, *x);
    if (f.size() >= 9) if (auto x = to_double(f[8])) d.wave_conf_pct = clampd(*x, 0.0, 100.0);
    if (f.size() >= 10) if (auto x = to_double(f[9])) d.hs_m = *x;
    if (f.size() >= 11) if (auto x = to_double(f[10])) d.tp_s = *x;
    if (f.size() >= 12) {
        std::string st = upper(f[11]);
        d.system_status = (st == "A" || st == "GOOD" || st == "OK") ? "INS GOOD" : "INS WARN";
    }
}

bool name_contains(const std::string& name, const std::string& needle) {
    return name.find(needle) != std::string::npos;
}

void parse_xdr(const std::vector<std::string>& f, InsData& d) {
    // $--XDR,type,value,unit,name groups of four after sentence id.
    // The supported sensor stream sends one measurement per sentence, for example:
    //   $IIXDR,A,-16.9,D,PTCH*79
    //   $IIXDR,A,15.4,D,ROLL*48
    //   $IIXDR,A,25.5,D,WAVAXIS*14
    //   $IIXDR,D,0.0000,M,DRT1*2A
    for (size_t i = 1; i + 3 < f.size(); i += 4) {
        auto val = to_double(f[i + 1]);
        if (!val) continue;

        const std::string measurement_type = upper(f[i]);
        const std::string unit = upper(f[i + 2]);
        const std::string name = upper(f[i + 3]);

        if (name_contains(name, "ROLL")) {
            d.roll_deg = *val;
        } else if (name_contains(name, "PITCH") || name_contains(name, "PTCH")) {
            d.pitch_deg = *val;
        } else if (name_contains(name, "YAW") || name_contains(name, "HDG") || name_contains(name, "HEAD")) {
            d.heading_deg = d.yaw_deg = wrap360(*val);
        } else if (name == "VHSPD" ||
                   ((name_contains(name, "HEAVE") || name_contains(name, "VERT")) &&
                    (name_contains(name, "VEL") || name_contains(name, "SPD") || name_contains(name, "SPEED")))) {
            d.heave_vel_mps = *val;
            d.heave_vel_available = true;
        } else if (name_contains(name, "HEAVE")) {
            d.heave_m = *val;
            d.heave_status = "GOOD";
        } else if (name_contains(name, "ROT")) {
            d.rot_deg_min = *val;
        } else if (name == "WAVAXIS" || name == "WAVDIR" || name_contains(name, "WAVE")) {
            apply_wave_degrees(d, *val);
        } else if (name == "HS") {
            d.hs_m = *val;
        } else if (name == "TP") {
            d.tp_s = *val;
        } else if (name == "IMUT" || name_contains(name, "IMU_TEMP") || name_contains(name, "IMUTEMP")) {
            d.temp_c = *val;
        } else if (name == "DRT1" && measurement_type == "D" && unit == "M") {
            d.heave_m = *val;
            d.heave_status = "GOOD";
        }
    }
}

void parse_txt(const std::vector<std::string>& f, InsData& d) {
    // $--TXT,total,msg,seq,text. The target wave sensor appends sign metadata such
    // as "WAVSGN=1 POL=1" after WAVAXIS/WAVDIR XDR sentences.
    for (size_t i = 4; i < f.size(); ++i) {
        std::string text = f[i];
        for (char& c : text) {
            if (c == ';' || c == ',') c = ' ';
        }

        std::istringstream tokens(text);
        std::string token;
        bool changed = false;
        while (tokens >> token) {
            const auto eq = token.find('=');
            if (eq == std::string::npos) continue;

            const std::string key = upper(token.substr(0, eq));
            const std::string value = token.substr(eq + 1);
            if (key == "WAVSGN" || key == "WAVSIGN" || key == "WAVE_SIGN") {
                d.wave_sign = direction_flag_to_sign(value);
                changed = true;
            } else if (key == "POL" || key == "POLARITY" || key == "WAVPOL" || key == "WAVE_POL") {
                d.wave_polarity = direction_flag_to_sign(value);
                changed = true;
            } else {
                set_status_field(d, key, value);
            }
        }
        if (changed) {
            reapply_wave_sign(d);
            d.wave_status = "GOOD";
        }
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
    } else if (type == "HDT" || type == "HDM" || type == "HDG") {
        if (f.size() > 1) if (auto x = to_double(f[1])) d.heading_deg = d.yaw_deg = wrap360(*x);
        if (type == "HDM") d.mag_status = "GOOD";
    } else if (type == "ROT") {
        if (f.size() > 1) if (auto x = to_double(f[1])) d.rot_deg_min = *x;
        if (f.size() > 2) d.system_status = upper(f[2]) == "A" ? "INS GOOD" : "INS WARN";
    } else if (type == "XDR") {
        parse_xdr(f, d);
    } else if (type == "TXT") {
        parse_txt(f, d);
    }
}

} // namespace ins_display::nmea
