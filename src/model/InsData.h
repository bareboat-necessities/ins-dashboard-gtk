#pragma once

#include <chrono>
#include <string>

namespace ins_display::model {

struct InsData {
    double heading_deg = 127.0;
    double yaw_deg = 127.0;
    double roll_deg = -6.2;
    double pitch_deg = 2.1;
    double heave_m = 0.18;
    double heave_vel_mps = 0.11;
    bool heave_vel_available = false;
    double rot_deg_min = 8.4;
    double wave_rel_deg = 45.0;
    double wave_rel_mag_deg = 45.0;
    int wave_sign = 1;
    int wave_polarity = 1;
    double wave_conf_pct = 78.0;
    double hs_m = 0.72;
    double tp_s = 4.8;
    double temp_c = 36.8;
    double sample_hz = 200.0;
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

} // namespace ins_display::model
