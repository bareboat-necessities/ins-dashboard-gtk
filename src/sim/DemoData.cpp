#include "sim/DemoData.h"

#include "util/MathUtil.h"

#include <chrono>
#include <cmath>
#include <mutex>

namespace ins_display::sim {

void update_demo(model::SharedModel& model, double t) {
    std::lock_guard<std::mutex> lock(model.mtx);
    auto& d = model.data;
    d.heading_deg = d.yaw_deg = util::wrap360(127.0 + 8.0 * std::sin(0.025 * t));
    d.roll_deg = -6.2 + 4.0 * std::sin(0.75 * t);
    d.pitch_deg = 2.1 + 2.0 * std::sin(0.53 * t + 1.1);
    d.heave_m = 0.24 * std::sin(1.35 * t) + 0.05 * std::sin(0.37 * t);
    d.heave_vel_mps = 0.24 * 1.35 * std::cos(1.35 * t) + 0.05 * 0.37 * std::cos(0.37 * t);
    d.rot_deg_min = 8.4 + 4.0 * std::sin(0.07 * t);
    d.wave_rel_deg = util::wrap360(45.0 + 10.0 * std::sin(0.05 * t));
    d.wave_conf_pct = 78.0 + 8.0 * std::sin(0.04 * t);
    d.hs_m = 0.72 + 0.08 * std::sin(0.03 * t);
    d.tp_s = 4.8 + 0.3 * std::sin(0.02 * t);
    d.temp_c = 36.8 + 0.2 * std::sin(0.01 * t);
    d.valid = true;
    d.last_update = std::chrono::steady_clock::now();
    d.last_sentence = "demo:// synthetic INS data";
}

} // namespace ins_display::sim
