#include "render/Screens.h"

#include "render/DrawUtil.h"
#include "render/Icons.h"
#include "util/MathUtil.h"
#include "util/StringUtil.h"

#include <cmath>
#include <functional>
#include <iterator>
#include <vector>

namespace ins_display::render {
namespace {
using util::clampd;
using util::DEG;
using util::fmt;
using util::fmt_signed;
using util::PI;
using util::wrap360;

void draw_triangle(cairo_t* cr, double x, double y, double size, double angle, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, angle);
    setc(cr, c);
    cairo_move_to(cr, 0, -size);
    cairo_line_to(cr, -0.65 * size, 0.75 * size);
    cairo_line_to(cr, 0.65 * size, 0.75 * size);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_restore(cr);
}


void draw_wave_direction_arrow(cairo_t* cr, double cx, double cy, double theta, double length, Color c) {
    const double tip_x = cx + length * std::cos(theta);
    const double tip_y = cy + length * std::sin(theta);

    cairo_save(cr);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    glow_line(cr, cx, cy, tip_x, tip_y, c, 8.0, 18.0);

    const double head = 30.0;
    setc(cr, c);
    cairo_move_to(cr, tip_x, tip_y);
    cairo_line_to(cr, tip_x - head * std::cos(theta - 0.46), tip_y - head * std::sin(theta - 0.46));
    cairo_line_to(cr, tip_x - head * std::cos(theta + 0.46), tip_y - head * std::sin(theta + 0.46));
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_restore(cr);

    const double wave_x = cx + (length - 62.0) * std::cos(theta);
    const double wave_y = cy + (length - 62.0) * std::sin(theta);
    cairo_save(cr);
    cairo_translate(cr, wave_x, wave_y);
    cairo_rotate(cr, theta + PI / 2.0);
    draw_wave_icon(cr, -34.0, -24.0, 0.48, c);
    cairo_restore(cr);
}

double estimated_heave_velocity(const HeaveHistory& hist, double now_s) {
    if (hist.empty()) return 0.0;

    const auto& newest = hist.back();
    const double min_dt = 0.05;
    const double target_window_s = 0.6;
    auto older = hist.end();

    for (auto it = hist.rbegin(); it != hist.rend(); ++it) {
        const double age = now_s - it->first;
        if (age >= target_window_s) {
            older = std::next(it).base();
            break;
        }
    }

    if (older == hist.end() && hist.size() >= 2) {
        older = hist.begin();
    }
    if (older == hist.end()) return 0.0;

    const double dt = newest.first - older->first;
    if (dt < min_dt) return 0.0;
    return (newest.second - older->second) / dt;
}

double displayed_heave_velocity(const model::InsData& d, const HeaveHistory& hist, double now_s) {
    return d.heave_vel_available ? d.heave_vel_mps : estimated_heave_velocity(hist, now_s);
}

void draw_background(cairo_t* cr) {
    fill_round_gradient(cr, 8, 8, 984, 984, 24, {0.013, 0.030, 0.055, 1.0}, {0.010, 0.018, 0.035, 1.0}, {0.10, 0.16, 0.26, 1.0}, 2.0);
    for (int i = 0; i < 5; ++i) {
        const double x = 90 + i * 200;
        line(cr, x, 30, x, 970, {0.10, 0.18, 0.28, 0.14}, 1.0);
    }
    for (int i = 0; i < 5; ++i) {
        const double y = 120 + i * 170;
        line(cr, 28, y, 972, y, {0.10, 0.18, 0.28, 0.12}, 1.0);
    }
}

void draw_title_bar(cairo_t* cr, const std::string& title, const std::string& subtitle = {}, const std::string& right = {}) {
    fill_round_gradient(cr, 20, 20, 960, 95, 18, {0.025, 0.080, 0.145, 0.96}, {0.012, 0.035, 0.070, 0.96}, LINE, 2.0);
    text_shadow(cr, title, 50, subtitle.empty() ? 67 : 52, subtitle.empty() ? 40 : 34, WHITE, "bold", 0, 0.5);
    if (!subtitle.empty()) {
        text(cr, subtitle, 50, 86, 22, CYAN, "bold", 0, 0.5);
    }
    if (!right.empty()) {
        draw_check_pill(cr, 690, 41, 250, 54, right, status_color(right));
    }
}

void draw_panel(cairo_t* cr, double x, double y, double w, double h, double r = 18.0) {
    fill_round_gradient(cr, x, y, w, h, r, {0.030, 0.092, 0.155, 0.92}, {0.018, 0.055, 0.105, 0.92}, LINE, 2.0);
}


void draw_curved_roll_scale(cairo_t* cr, double cx, double cy, double radius, int side) {
    // Side roll scales share the same center as the boat/attitude ball, so
    // their arches are concentric with the vessel symbol rather than being
    // small, separate scales beside it.
    const double ar = radius + 58.0;

    // Define one side, then mirror the angles across the vertical centerline.
    // This keeps the port and starboard scales the same size and puts the
    // matching degree marks at the same height on both sides.
    const double port_top = 2.22;
    const double port_bottom = 4.06;
    const double top = (side < 0) ? port_top : PI - port_top;
    const double bottom = (side < 0) ? port_bottom : PI - port_bottom;
    const bool clockwise = side > 0;

    auto stroke_arc = [&](double start, double end, Color c, double width) {
        // Start each arc as an independent path. Cairo connects an arc to an
        // existing current point with a straight segment, which can leave
        // visible diagonal artifacts from nearby scale labels/ticks.
        cairo_new_path(cr);
        setc(cr, c);
        cairo_set_line_width(cr, width);
        if (clockwise) {
            cairo_arc_negative(cr, cx, cy, ar, start, end);
        } else {
            cairo_arc(cr, cx, cy, ar, start, end);
        }
        cairo_stroke(cr);
    };

    stroke_arc(top, bottom, {0.85, 0.90, 0.97, 0.45}, 2.0);
    stroke_arc(top, top + (clockwise ? -0.32 : 0.32), RED, 3.2);
    stroke_arc(bottom + (clockwise ? 0.32 : -0.32), bottom, GREEN, 3.2);

    auto tick_angle = [&](double deg) {
        // +60 at top, 0 middle, -60 bottom.
        const double t = (60.0 - deg) / 120.0;
        return top + t * (bottom - top);
    };

    for (int deg = -60; deg <= 60; deg += 5) {
        const double ang = tick_angle(static_cast<double>(deg));
        const double outer = ar;
        const double inner = outer - ((deg % 30 == 0) ? 20.0 : (deg % 10 == 0 ? 13.0 : 7.0));

        line(cr,
             cx + inner * std::cos(ang), cy + inner * std::sin(ang),
             cx + outer * std::cos(ang), cy + outer * std::sin(ang),
             WHITE,
             (deg % 30 == 0) ? 2.2 : 1.1);

        if (deg % 30 == 0) {
            const double tx = cx + (ar - 40.0) * std::cos(ang);
            const double ty = cy + (ar - 40.0) * std::sin(ang);
            text_shadow(cr, std::to_string(std::abs(deg)), tx, ty, 20, WHITE, "bold", 0.5, 0.5);
        }
    }

    const double ang0 = tick_angle(0.0);
    draw_triangle(cr,
                  cx + (ar - 2.0) * std::cos(ang0),
                  cy + (ar - 2.0) * std::sin(ang0),
                  12,
                  ang0 + ((side < 0) ? PI / 2.0 : -PI / 2.0),
                  WHITE);
}

void draw_status_row(cairo_t* cr, int idx, const std::string& label, const std::string& value, Color vc,
                     const std::function<void(cairo_t*, double, double, double, Color)>& icon) {
    const double y = 140 + idx * 82;
    draw_panel(cr, 38, y, 924, 65, 12);
    line(cr, 148, y + 10, 148, y + 55, {1, 1, 1, 0.10}, 1.5);
    line(cr, 520, y + 10, 520, y + 55, {1, 1, 1, 0.10}, 1.5);
    icon(cr, 92, y + 34, 0.72, CYAN);
    text_shadow(cr, label, 185, y + 34, 31, WHITE, "bold", 0, 0.5);
    cairo_new_path(cr);
    setc(cr, vc);
    cairo_arc(cr, 575, y + 34, 11, 0, 2 * PI);
    cairo_fill(cr);
    text_shadow(cr, value, 625, y + 34, 34, vc, "bold", 0, 0.5);
}

} // namespace

void draw_primary(cairo_t* cr, const model::InsData& d) {
    draw_background(cr);

    fill_round_gradient(cr, 20, 20, 960, 100, 18, {0.030, 0.095, 0.165, 0.96}, {0.015, 0.042, 0.078, 0.96}, LINE, 2);
    text_shadow(cr, "HDG", 60, 72, 40, WHITE, "bold", 0, 0.5);
    text_shadow(cr, fmt(d.heading_deg, 0) + "°", 185, 72, 72, CYAN, "bold", 0, 0.5);
    line(cr, 500, 28, 500, 112, {1, 1, 1, 0.12}, 2);
    const double rot_rpm_primary = d.rot_deg_min / 360.0;
    text_shadow(cr, "ROT", 555, 72, 40, WHITE, "bold", 0, 0.5);
    text_shadow(cr, fmt_signed(rot_rpm_primary, 3), 682, 72, 58, CYAN, "bold", 0, 0.5);
    text(cr, "RPM", 865, 84, 30, CYAN, "bold", 0, 0.5);

    const double cx = 500, cy = 405, r = 290;
    fill_circle_gradient(cr, cx, cy, r + 15, {0.03, 0.10, 0.19, 1.0}, {0.008, 0.018, 0.036, 1.0}, LINE, 5.0);
    fill_circle_gradient(cr, cx, cy, r, {0.02, 0.12, 0.24, 0.95}, {0.01, 0.03, 0.06, 0.95}, {0.18, 0.25, 0.34, 0.8}, 1.5);

    cairo_save(cr);
    cairo_new_path(cr);
    cairo_arc(cr, cx, cy, r - 10, 0, 2 * PI);
    cairo_clip(cr);
    cairo_translate(cr, cx, cy);
    cairo_rotate(cr, -d.roll_deg * DEG);
    const double pitch_px = clampd(d.pitch_deg, -25, 25) * 5.6;
    setc(cr, {0.035, 0.44, 0.92, 1.0});
    cairo_rectangle(cr, -r - 30, -r - 30 + pitch_px, 2 * r + 60, r + 30 - pitch_px);
    cairo_fill(cr);
    setc(cr, {0.015, 0.060, 0.130, 1.0});
    cairo_rectangle(cr, -r - 30, pitch_px, 2 * r + 60, r + 30);
    cairo_fill(cr);
    glow_line(cr, -r, pitch_px, r, pitch_px, WHITE, 3.2, 12.0);
    for (int p = -30; p <= 30; p += 10) {
        if (p == 0) continue;
        const double y = pitch_px - p * 5.6;
        const double len = (std::abs(p) % 20 == 0) ? 190 : 120;
        line(cr, -len / 2, y, len / 2, y, WHITE, 2);
        text_shadow(cr, std::to_string(std::abs(p)), -len / 2 - 36, y, 24, WHITE, "bold", 0.5, 0.5);
        text_shadow(cr, std::to_string(std::abs(p)), len / 2 + 36, y, 24, WHITE, "bold", 0.5, 0.5);
    }
    cairo_restore(cr);

    for (int a = -60; a <= 60; a += 5) {
        const double theta = (a - 90) * DEG;
        const double inner = r - ((a % 30 == 0) ? 42 : 24);
        const double outer = r - 5;
        line(cr, cx + inner * std::cos(theta), cy + inner * std::sin(theta),
             cx + outer * std::cos(theta), cy + outer * std::sin(theta), WHITE, a % 10 == 0 ? 2.5 : 1.2);
        if (a != 0 && a % 30 == 0) {
            text_shadow(cr, std::to_string(std::abs(a)),
                        cx + (inner - 36) * std::cos(theta), cy + (inner - 36) * std::sin(theta),
                        23, WHITE, "bold", 0.5, 0.5);
        }
    }
    draw_curved_roll_scale(cr, cx, cy, r, -1);
    draw_curved_roll_scale(cr, cx, cy, r, +1);
    draw_triangle(cr, cx, cy - r - 13, 16, PI, CYAN);
    draw_triangle(cr, cx, cy + r + 13, 16, 0, CYAN);
    draw_boat_front(cr, cx, cy + 8, 0.9, WHITE);

    draw_panel(cr, 25, 708, 465, 106);
    draw_panel(cr, 510, 708, 465, 106);
    draw_roll_icon(cr, 96, 756, 1.0, CYAN);
    draw_pitch_icon(cr, 588, 748, 0.95, CYAN);
    text_shadow(cr, "ROLL", 180, 762, 32, WHITE, "bold", 0, 0.5);
    text_shadow(cr, fmt_signed(d.roll_deg, 1) + "°", 300, 762, 56, CYAN, "bold", 0, 0.5);
    text_shadow(cr, "PITCH", 655, 762, 32, WHITE, "bold", 0, 0.5);
    text_shadow(cr, fmt_signed(d.pitch_deg, 1) + "°", 795, 762, 56, CYAN, "bold", 0, 0.5);

    draw_panel(cr, 25, 830, 950, 95);
    draw_heave_icon(cr, 92, 868, 0.9, CYAN);
    text_shadow(cr, "HEAVE", 180, 880, 30, WHITE, "bold", 0, 0.5);
    text_shadow(cr, fmt(d.heave_m, 2), 300, 882, 52, GREEN, "bold", 0, 0.5);
    text(cr, "m", 420, 886, 28, WHITE, "bold", 0, 0.5);
    text_shadow(cr, d.heave_vel_mps >= 0 ? "↑" : "↓", 498, 882, 64, GREEN, "bold", 0.5, 0.5);
    line(cr, 530, 845, 530, 910, {1, 1, 1, 0.10}, 2);
    draw_wave_circle_icon(cr, 585, 878, 0.85, CYAN);
    text_shadow(cr, "WAVES", 650, 862, 30, WHITE, "bold", 0, 0.5);
    text_shadow(cr, fmt(d.wave_rel_deg, 0) + "°", 635, 900, 48, CYAN, "bold", 0, 0.5);
    text(cr, rel_wave_name(d.wave_rel_deg), 780, 900, 28, WHITE, "bold", 0, 0.5);
    draw_check_pill(cr, 345, 940, 310, 45, d.system_status, status_color(d.system_status));
}

void draw_heave(cairo_t* cr, const model::InsData& d, const HeaveHistory& hist, double now_s) {
    const double heave_vel_mps = displayed_heave_velocity(d, hist, now_s);

    draw_background(cr);
    draw_title_bar(cr, "HEAVE", "VERTICAL MOTION", d.system_status);

    draw_panel(cr, 30, 140, 940, 250);
    line(cr, 210, 165, 210, 360, {1, 1, 1, 0.10}, 2);
    draw_vertical_motion_icon(cr, 122, 260, 0.95, CYAN);
    text_shadow(cr, fmt(d.heave_m, 2), 315, 255, 122, CYAN, "bold", 0, 0.5);
    text(cr, "m", 768, 282, 50, CYAN, "bold", 0, 0.5);
    text_shadow(cr, heave_vel_mps >= 0 ? "↑" : "↓", 885, 265, 105, GREEN, "bold", 0.5, 0.5);
    text(cr, heave_vel_mps >= 0 ? "RISING" : "FALLING", 500, 340, 34, CYAN, "bold", 0.5, 0.5);
    line(cr, 365, 350, 455, 350, {0.50, 0.85, 1.0, 0.35}, 1.5);
    line(cr, 545, 350, 635, 350, {0.50, 0.85, 1.0, 0.35}, 1.5);

    draw_panel(cr, 30, 430, 940, 82, 16);
    text_shadow(cr, "VERTICAL SPEED", 120, 472, 31, WHITE, "bold", 0, 0.5);
    text_shadow(cr, fmt_signed(heave_vel_mps, 2), 520, 472, 55, CYAN, "bold", 0, 0.5);
    text(cr, "m/s", 730, 476, 32, CYAN, "bold", 0, 0.5);
    text_shadow(cr, heave_vel_mps >= 0 ? "↑" : "↓", 900, 472, 52, GREEN, "bold", 0.5, 0.5);

    const double gx = 55, gy = 545, gw = 890, gh = 260;
    fill_round_gradient(cr, gx, gy, gw, gh, 18, {0.010, 0.030, 0.060, 0.92}, {0.007, 0.018, 0.040, 0.92}, LINE, 2);
    auto ymap = [&](double h) { return gy + gh / 2.0 - clampd(h, -0.6, 0.6) / 0.6 * (gh / 2.0 - 20); };
    line(cr, gx + 80, ymap(0), gx + gw - 25, ymap(0), MUTED, 1.5);
    text(cr, "+0.5 m", gx + 22, ymap(0.5), 22, WHITE, "normal", 0, 0.5);
    text(cr, "0", gx + 72, ymap(0), 22, WHITE, "normal", 1, 0.5);
    text(cr, "-0.5 m", gx + 22, ymap(-0.5), 22, WHITE, "normal", 0, 0.5);
    for (int i = 0; i <= 4; ++i) {
        const double x = gx + 80 + i * (gw - 120) / 4.0;
        line(cr, x, gy + 20, x, gy + gh - 50, {0.25, 0.35, 0.48, 0.40}, 1);
    }

    setc(cr, {0.02, 0.60, 1.0, 0.20});
    bool have = false;
    for (auto [t, h] : hist) {
        const double age = now_s - t;
        if (age < 0 || age > 20) continue;
        const double x = gx + 80 + (20.0 - age) / 20.0 * (gw - 120);
        const double y = ymap(h);
        if (!have) { cairo_move_to(cr, x, y); have = true; }
        else cairo_line_to(cr, x, y);
    }
    if (have) {
        cairo_line_to(cr, gx + gw - 40, ymap(0));
        cairo_line_to(cr, gx + 80, ymap(0));
        cairo_close_path(cr);
        cairo_fill(cr);
    }
    setc(cr, CYAN);
    cairo_set_line_width(cr, 3.5);
    have = false;
    double last_x = 0, last_y = 0;
    for (auto [t, h] : hist) {
        const double age = now_s - t;
        if (age < 0 || age > 20) continue;
        const double x = gx + 80 + (20.0 - age) / 20.0 * (gw - 120);
        const double y = ymap(h);
        last_x = x; last_y = y;
        if (!have) { cairo_move_to(cr, x, y); have = true; }
        else cairo_line_to(cr, x, y);
    }
    if (have) cairo_stroke(cr);
    if (have) {
        cairo_new_path(cr);
        setc(cr, CYAN);
        cairo_arc(cr, last_x, last_y, 5, 0, 2 * PI);
        cairo_fill(cr);
    }
    text(cr, "LAST 20 SECONDS", 500, gy + gh - 18, 24, CYAN, "bold", 0.5, 0.5);

    draw_panel(cr, 30, 835, 455, 105);
    draw_panel(cr, 515, 835, 455, 105);
    draw_wave_circle_icon(cr, 86, 888, 0.82, CYAN);
    text_shadow(cr, "Hs", 180, 890, 32, WHITE, "bold", 0, 0.5);
    text_shadow(cr, fmt(d.hs_m, 2), 250, 892, 58, CYAN, "bold", 0, 0.5);
    text(cr, "m", 415, 898, 30, CYAN, "bold", 0, 0.5);
    draw_clock_icon(cr, 590, 888, 0.72, CYAN);
    text_shadow(cr, "Tp", 660, 890, 32, WHITE, "bold", 0, 0.5);
    text_shadow(cr, fmt(d.tp_s, 1), 750, 892, 58, CYAN, "bold", 0, 0.5);
    text(cr, "s", 895, 898, 30, CYAN, "bold", 0, 0.5);
}

void draw_wave(cairo_t* cr, const model::InsData& d) {
    draw_background(cr);
    draw_title_bar(cr, "WAVE DIRECTION", "RELATIVE TO VESSEL HEADING", "CONF " + fmt(d.wave_conf_pct, 0) + "%");

    const double cx = 500, cy = 470, r = 300;
    fill_circle_gradient(cr, cx, cy, r + 12, {0.018, 0.07, 0.14, 1.0}, {0.008, 0.022, 0.040, 1.0}, LINE, 3.0);
    dashed_circle(cr, cx, cy, 220, {0.12, 0.72, 1.0, 0.36}, 1.5, 6, 7);
    dashed_circle(cr, cx, cy, 160, {0.12, 0.72, 1.0, 0.30}, 1.4, 5, 6);
    dashed_circle(cr, cx, cy, 100, {0.12, 0.72, 1.0, 0.24}, 1.2, 4, 6);
    line(cr, cx - r, cy, cx + r, cy, {0.02, 0.45, 1, 0.45}, 1.5);
    line(cr, cx, cy - r, cx, cy + r, {0.02, 0.45, 1, 0.45}, 1.5);

    for (int a = 0; a < 360; a += 5) {
        const double theta = (a - 90) * DEG;
        const double inner = r - ((a % 30 == 0) ? 18 : 10);
        line(cr, cx + inner * std::cos(theta), cy + inner * std::sin(theta),
             cx + r * std::cos(theta), cy + r * std::sin(theta), CYAN, a % 30 == 0 ? 2.2 : 1.0);
        if (a % 30 == 0) {
            const double tx = cx + (r + 34) * std::cos(theta);
            const double ty = cy + (r + 34) * std::sin(theta);
            text(cr, std::to_string(a) + "°", tx, ty, 22, WHITE, "normal", 0.5, 0.5);
        }
    }

    text_shadow(cr, "BOW", cx, cy - r - 48, 28, WHITE, "bold", 0.5, 0.5);
    text_shadow(cr, "STERN", cx, cy + r + 48, 28, WHITE, "bold", 0.5, 0.5);
    text_shadow(cr, "PORT", cx - r - 90, cy, 28, WHITE, "bold", 0.5, 0.5);
    text_shadow(cr, "STBD", cx + r + 90, cy, 28, WHITE, "bold", 0.5, 0.5);
    draw_triangle(cr, cx, cy - r - 20, 12, PI, CYAN);
    draw_triangle(cr, cx, cy + r + 20, 12, 0, CYAN);
    draw_triangle(cr, cx - r - 18, cy, 12, PI / 2, CYAN);
    draw_triangle(cr, cx + r + 18, cy, 12, -PI / 2, CYAN);
    const double theta = (d.wave_rel_deg - 90.0) * DEG;
    draw_wave_direction_arrow(cr, cx, cy, theta, 235.0, CYAN);
    draw_boat_top(cr, cx, cy, 0.95, WHITE);

    draw_panel(cr, 20, 825, 960, 140);
    text_shadow(cr, fmt(d.wave_rel_deg, 0) + "°", 70, 895, 82, CYAN, "bold", 0, 0.5);
    text(cr, "REL", 335, 912, 34, CYAN, "bold", 0, 0.5);
    line(cr, 500, 845, 500, 945, {1, 1, 1, 0.10}, 2);
    draw_wave_icon(cr, 535, 875, 0.62, CYAN);
    text_shadow(cr, "FROM", 650, 872, 32, WHITE, "bold", 0, 0.5);
    text_shadow(cr, rel_wave_name(d.wave_rel_deg), 650, 922, 50, CYAN, "bold", 0, 0.5);
}

void draw_rot(cairo_t* cr, const model::InsData& d) {
    draw_background(cr);

    // Custom centered title bar closer to the design mockup.
    fill_round_gradient(cr, 20, 20, 960, 88, 18, {0.020, 0.060, 0.115, 0.96}, {0.012, 0.030, 0.060, 0.96}, LINE, 2.0);
    line(cr, 70, 64, 320, 64, {1, 1, 1, 0.10}, 2.0);
    line(cr, 680, 64, 930, 64, {1, 1, 1, 0.10}, 2.0);
    text_shadow(cr, "RATE OF TURN", 500, 60, 44, WHITE, "bold", 0.5, 0.5);

    text_shadow(cr, "PORT", 78, 198, 38, RED, "bold", 0, 0.5);
    text_shadow(cr, "STBD", 815, 198, 38, GREEN, "bold", 0, 0.5);

    constexpr double ROT_RPM_LIMIT = 2.0;
    constexpr double ROT_RPM_TICK_STEP = 0.1;
    constexpr double ROT_RPM_LABEL_STEP = 0.5;

    const double rot_rpm = d.rot_deg_min / 360.0;
    const double cx = 500, cy = 545, r = 390;

    // Main semicircular gauge body - close across the diameter, not to the center.
    cairo_new_path(cr);
    cairo_arc(cr, cx, cy, r, PI, 2 * PI);
    cairo_close_path(cr);
    cairo_pattern_t* pat = cairo_pattern_create_radial(cx, cy + 40, 40, cx, cy + 10, r + 20);
    cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.05, 0.23, 0.55, 0.96);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, 0.01, 0.06, 0.16, 0.98);
    cairo_set_source(cr, pat);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(pat);
    setc(cr, {0.18, 0.24, 0.34, 0.85});
    cairo_set_line_width(cr, 3.5);
    cairo_stroke(cr);

    // Inner blue rim.
    cairo_new_path(cr);
    setc(cr, {0.04, 0.62, 1.0, 0.95});
    cairo_set_line_width(cr, 4.0);
    cairo_arc(cr, cx, cy, r - 18, PI, 2 * PI);
    cairo_stroke(cr);

    // Colored edge bands.
    cairo_new_path(cr);
    setc(cr, RED);
    cairo_set_line_width(cr, 6.0);
    cairo_arc(cr, cx, cy, r - 26, 1.03 * PI, 1.18 * PI);
    cairo_stroke(cr);
    cairo_new_path(cr);
    setc(cr, GREEN);
    cairo_arc(cr, cx, cy, r - 26, -0.18 * PI, -0.03 * PI);
    cairo_stroke(cr);

    // Tick marks and numeric labels.
    const int tick_count = static_cast<int>(std::round(ROT_RPM_LIMIT / ROT_RPM_TICK_STEP));
    const int label_interval = static_cast<int>(std::round(ROT_RPM_LABEL_STEP / ROT_RPM_TICK_STEP));
    for (int vi = -tick_count; vi <= tick_count; ++vi) {
        const double v = vi * ROT_RPM_TICK_STEP;
        const bool labeled_tick = (vi % label_interval == 0);
        const double angle = (-90.0 + (v / ROT_RPM_LIMIT) * 90.0) * DEG;
        const double outer = r - 18;
        const double inner = outer - (labeled_tick ? 30 : 14);
        line(cr, cx + inner * std::cos(angle), cy + inner * std::sin(angle),
             cx + outer * std::cos(angle), cy + outer * std::sin(angle),
             WHITE, labeled_tick ? 2.4 : 1.4);
        if (labeled_tick) {
            const double tx = cx + (outer - 74) * std::cos(angle);
            const double ty = cy + (outer - 74) * std::sin(angle);
            std::string label = (vi == 0) ? "0" : fmt(std::abs(v), 1);
            text_shadow(cr, label, tx, ty, vi == 0 ? 34 : 24, WHITE, "bold", 0.5, 0.5);
        }
    }
    text_shadow(cr, "RPM", cx, cy - 220, 34, CYAN, "bold", 0.5, 0.5);
    draw_triangle(cr, cx, cy - r - 4, 18, PI, CYAN);

    // Pointer as a filled needle polygon.
    const double val = clampd(rot_rpm, -ROT_RPM_LIMIT, ROT_RPM_LIMIT);
    const double angle = (-90.0 + (val / ROT_RPM_LIMIT) * 90.0) * DEG;
    const double tip_r = r - 100;
    cairo_save(cr);
    cairo_translate(cr, cx, cy);
    cairo_rotate(cr, angle + PI / 2.0);
    setc(cr, {1.0, 1.0, 1.0, 0.18});
    cairo_move_to(cr, -12, 0);
    cairo_line_to(cr, 12, 0);
    cairo_line_to(cr, 8, -tip_r + 14);
    cairo_line_to(cr, -8, -tip_r + 14);
    cairo_close_path(cr);
    cairo_fill(cr);
    setc(cr, {0.95, 0.96, 0.98, 0.98});
    cairo_move_to(cr, -10, 0);
    cairo_line_to(cr, 10, 0);
    cairo_line_to(cr, 9, -tip_r);
    cairo_line_to(cr, -9, -tip_r + 8);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_restore(cr);

    fill_circle_gradient(cr, cx, cy, 46, {0.14, 0.18, 0.28, 1.0}, {0.07, 0.09, 0.14, 1.0}, LINE, 2.0);
    fill_circle_gradient(cr, cx, cy, 34, {0.18, 0.22, 0.34, 1.0}, {0.10, 0.13, 0.20, 1.0}, {0.25, 0.35, 0.50, 0.6}, 1.0);

    draw_panel(cr, 50, 610, 900, 135);
    text_shadow(cr, fmt_signed(rot_rpm, 3), 118, 680, 82, CYAN, "bold", 0, 0.5);
    text(cr, "RPM", 430, 693, 42, CYAN, "bold", 0, 0.5);
    line(cr, 625, 635, 625, 720, {1, 1, 1, 0.10}, 2);
    text_shadow(cr, "TURNING", 700, 656, 35, WHITE, "bold", 0, 0.5);
    text_shadow(cr, rot_rpm >= 0 ? "STBD" : "PORT", 700, 705, 55, rot_rpm >= 0 ? GREEN : RED, "bold", 0, 0.5);

    draw_panel(cr, 50, 765, 900, 105);
    text_shadow(cr, "HDG TREND", 70, 815, 28, WHITE, "bold", 0, 0.5);
    const double h1 = wrap360(d.heading_deg - d.rot_deg_min / 6.0);
    const double h3 = wrap360(d.heading_deg + d.rot_deg_min / 6.0);
    text_shadow(cr, fmt(h1, 0) + "°", 300, 805, 42, CYAN, "bold", 0.5, 0.5);
    text(cr, "-10 sec", 300, 850, 25, CYAN, "bold", 0.5, 0.5);
    text_shadow(cr, "→", 430, 805, 40, WHITE, "bold", 0.5, 0.5);
    text_shadow(cr, fmt(d.heading_deg, 0) + "°", 560, 805, 42, CYAN, "bold", 0.5, 0.5);
    text(cr, "NOW", 560, 850, 25, CYAN, "bold", 0.5, 0.5);
    text_shadow(cr, "→", 690, 805, 40, WHITE, "bold", 0.5, 0.5);
    text_shadow(cr, fmt(h3, 0) + "°", 820, 805, 42, CYAN, "bold", 0.5, 0.5);
    text(cr, "+10 sec", 820, 850, 25, CYAN, "bold", 0.5, 0.5);
    draw_check_pill(cr, 345, 915, 310, 50, d.system_status, status_color(d.system_status));
}

void draw_status(cairo_t* cr, const model::InsData& d) {
    draw_background(cr);
    fill_round_gradient(cr, 20, 20, 960, 945, 18, {0.016, 0.034, 0.062, 0.96}, {0.010, 0.020, 0.042, 0.96}, LINE, 2);
    text_shadow(cr, "INS STATUS", 60, 75, 55, WHITE, "bold", 0, 0.5);
    draw_check_pill(cr, 590, 42, 350, 60, "SYSTEM HEALTHY", GREEN);

    draw_status_row(cr, 0, "ATTITUDE", d.attitude_status, status_color(d.attitude_status), draw_attitude_icon);
    draw_status_row(cr, 1, "HEAVE", d.heave_status, status_color(d.heave_status), draw_heave_status_icon);
    draw_status_row(cr, 2, "WAVE DIR", d.wave_status, status_color(d.wave_status), draw_compass_icon);
    draw_status_row(cr, 3, "MAG", d.mag_status, status_color(d.mag_status), draw_magnet_icon);
    draw_status_row(cr, 4, "GYRO BIAS", d.gyro_bias_status, status_color(d.gyro_bias_status), draw_gyro_icon);
    draw_status_row(cr, 5, "ACC BIAS", d.acc_bias_status, status_color(d.acc_bias_status), draw_accel_icon);
    draw_status_row(cr, 6, "TEMP", fmt(d.temp_c, 1) + " °C", CYAN, draw_thermo_icon);
    draw_status_row(cr, 7, "SAMPLE", fmt(d.sample_hz, 0) + " Hz", CYAN, draw_samplerate_icon);
    draw_status_row(cr, 8, "MAG RATE", fmt(d.mag_rate_hz, 0) + " Hz", CYAN, draw_bars_icon);
}

} // namespace ins_display::render
