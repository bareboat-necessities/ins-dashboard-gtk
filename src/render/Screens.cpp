#include "render/Screens.h"

#include "render/DrawUtil.h"
#include "render/Icons.h"
#include "util/MathUtil.h"
#include "util/StringUtil.h"

#include <cmath>

namespace ins_display::render {
namespace {
using util::clampd;
using util::DEG;
using util::fmt;
using util::fmt_signed;
using util::PI;
using util::wrap360;

void draw_title_bar(cairo_t* cr, const std::string& title, const std::string& right = "") {
    fill_round(cr, 20, 20, 960, 92, 18, {0.015, 0.030, 0.055, 0.92}, LINE, 2);
    text(cr, title, 55, 66, 38, WHITE, "bold", 0, 0.5);
    if (!right.empty()) draw_check_pill(cr, 670, 38, 270, 54, right, status_color(right));
}

void draw_status_row(cairo_t* cr, int idx, const std::string& label, const std::string& value, Color vc) {
    const double y = 140 + idx * 82;
    fill_round(cr, 38, y, 924, 65, 12, PANEL2, LINE, 1.5);
    text(cr, label, 205, y + 34, 31, WHITE, "bold", 0, 0.5);
    line(cr, 520, y + 12, 520, y + 53, LINE, 1.5);
    setc(cr, vc);
    cairo_arc(cr, 575, y + 34, 12, 0, 2 * PI);
    cairo_fill(cr);
    text(cr, value, 625, y + 34, 34, vc, "bold", 0, 0.5);
}

double signed_relative_degrees(double deg) {
    deg = wrap360(deg);
    return deg > 180.0 ? deg - 360.0 : deg;
}

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

} // namespace

void draw_primary(cairo_t* cr, const model::InsData& d) {
    fill_round(cr, 20, 20, 960, 100, 18, {0.015, 0.030, 0.055, 0.92}, LINE, 2);
    text(cr, "HDG", 60, 72, 40, WHITE, "bold", 0, 0.5);
    text(cr, fmt(d.heading_deg, 0) + "°", 185, 72, 72, CYAN, "bold", 0, 0.5);
    line(cr, 500, 28, 500, 112, LINE, 2);
    text(cr, "ROT", 555, 72, 40, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(d.rot_deg_min, 0) + "°", 682, 72, 72, CYAN, "bold", 0, 0.5);
    text(cr, "/min", 850, 85, 32, CYAN, "bold", 0, 0.5);

    const double cx = 500, cy = 398, r = 300;
    setc(cr, {0.02, 0.31, 0.78, 0.92});
    cairo_arc(cr, cx, cy, r, 0, 2 * PI);
    cairo_fill_preserve(cr);
    setc(cr, LINE);
    cairo_set_line_width(cr, 5);
    cairo_stroke(cr);

    cairo_save(cr);
    cairo_arc(cr, cx, cy, r - 10, 0, 2 * PI);
    cairo_clip(cr);
    cairo_translate(cr, cx, cy);
    cairo_rotate(cr, -d.roll_deg * DEG);
    const double pitch_px = clampd(d.pitch_deg, -25, 25) * 5.5;
    setc(cr, {0.02, 0.42, 0.92, 1});
    cairo_rectangle(cr, -r - 30, -r - 30 + pitch_px, 2 * r + 60, r + 30 - pitch_px);
    cairo_fill(cr);
    setc(cr, {0.01, 0.05, 0.14, 1});
    cairo_rectangle(cr, -r - 30, pitch_px, 2 * r + 60, r + 30);
    cairo_fill(cr);
    line(cr, -r, pitch_px, r, pitch_px, WHITE, 3.5);
    for (int p = -30; p <= 30; p += 10) {
        if (p == 0) continue;
        const double y = pitch_px - p * 5.5;
        const double len = (std::abs(p) % 20 == 0) ? 190 : 120;
        line(cr, -len / 2, y, len / 2, y, WHITE, 2);
        text(cr, std::to_string(std::abs(p)), -len / 2 - 35, y, 24, WHITE, "bold", 0.5, 0.5);
        text(cr, std::to_string(std::abs(p)), len / 2 + 35, y, 24, WHITE, "bold", 0.5, 0.5);
    }
    cairo_restore(cr);

    for (int a = -60; a <= 60; a += 5) {
        const double theta = (a - 90) * DEG;
        const double inner = r - ((a % 30 == 0) ? 44 : 26);
        const double outer = r - 5;
        line(cr, cx + inner * std::cos(theta), cy + inner * std::sin(theta),
             cx + outer * std::cos(theta), cy + outer * std::sin(theta), WHITE, a % 10 == 0 ? 2.5 : 1.3);
        if (a != 0 && a % 30 == 0) {
            text(cr, std::to_string(std::abs(a)), cx + (inner - 36) * std::cos(theta), cy + (inner - 36) * std::sin(theta),
                 23, WHITE, "bold", 0.5, 0.5);
        }
    }

    draw_triangle(cr, cx, cy - r - 12, 18, PI, CYAN);
    draw_triangle(cr, cx, cy + r + 12, 18, 0, CYAN);
    draw_boat_front(cr, cx, cy + 8, 0.9, WHITE);

    fill_round(cr, 25, 708, 465, 106, 18, PANEL2, LINE, 2);
    fill_round(cr, 510, 708, 465, 106, 18, PANEL2, LINE, 2);
    draw_roll_icon(cr, 105, 760, 1.0, CYAN);
    draw_pitch_icon(cr, 590, 748, 0.95, CYAN);
    text(cr, "ROLL", 180, 762, 32, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(d.roll_deg, 1) + "°", 300, 762, 56, CYAN, "bold", 0, 0.5);
    text(cr, "PITCH", 655, 762, 32, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(d.pitch_deg, 1) + "°", 795, 762, 56, CYAN, "bold", 0, 0.5);

    fill_round(cr, 25, 830, 950, 95, 18, PANEL2, LINE, 2);
    draw_wave_icon(cr, 80, 870, 0.55, CYAN);
    text(cr, "HEAVE", 180, 880, 30, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(d.heave_m, 2), 300, 882, 52, GREEN, "bold", 0, 0.5);
    text(cr, "m", 430, 886, 28, WHITE, "bold", 0, 0.5);
    text(cr, d.heave_vel_mps >= 0 ? "↑" : "↓", 500, 882, 64, GREEN, "bold", 0.5, 0.5);
    line(cr, 530, 845, 530, 910, LINE, 2);
    text(cr, "WAVES", 620, 862, 30, WHITE, "bold", 0, 0.5);
    text(cr, fmt_signed(signed_relative_degrees(d.wave_rel_deg), 0) + "°", 635, 900, 48, CYAN, "bold", 0, 0.5);
    text(cr, rel_wave_name(d.wave_rel_deg), 780, 900, 28, WHITE, "bold", 0, 0.5);
    draw_check_pill(cr, 345, 940, 310, 45, d.system_status, status_color(d.system_status));
}

void draw_heave(cairo_t* cr, const model::InsData& d, const HeaveHistory& hist, double now_s) {
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

    const double gx = 55, gy = 545, gw = 890, gh = 260;
    fill_round(cr, gx, gy, gw, gh, 18, {0.008, 0.025, 0.055, 0.85}, LINE, 2);
    auto ymap = [&](double h) { return gy + gh / 2.0 - clampd(h, -0.6, 0.6) / 0.6 * (gh / 2.0 - 20); };
    line(cr, gx + 80, ymap(0), gx + gw - 25, ymap(0), MUTED, 1.5);
    text(cr, "+0.5 m", gx + 22, ymap(0.5), 22, WHITE, "normal", 0, 0.5);
    text(cr, "0", gx + 72, ymap(0), 22, WHITE, "normal", 1, 0.5);
    text(cr, "-0.5 m", gx + 22, ymap(-0.5), 22, WHITE, "normal", 0, 0.5);

    for (int i = 0; i <= 4; ++i) {
        const double x = gx + 80 + i * (gw - 120) / 4.0;
        line(cr, x, gy + 20, x, gy + gh - 50, {0.25, 0.35, 0.48, 0.50}, 1);
    }

    setc(cr, {0.02, 0.60, 1.0, 0.22});
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
    for (auto [t, h] : hist) {
        const double age = now_s - t;
        if (age < 0 || age > 20) continue;
        const double x = gx + 80 + (20.0 - age) / 20.0 * (gw - 120);
        const double y = ymap(h);
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

void draw_wave(cairo_t* cr, const model::InsData& d) {
    fill_round(cr, 20, 20, 960, 105, 18, {0.015, 0.030, 0.055, 0.92}, LINE, 2);
    text(cr, "WAVE DIRECTION", 50, 55, 36, WHITE, "bold", 0, 0.5);
    text(cr, "RELATIVE TO VESSEL HEADING", 50, 93, 24, CYAN, "bold", 0, 0.5);
    draw_check_pill(cr, 690, 43, 250, 55, "CONF " + fmt(d.wave_conf_pct, 0) + "%", GREEN);

    const double cx = 500, cy = 480, r = 300;
    setc(cr, LINE);
    cairo_set_line_width(cr, 3);
    cairo_arc(cr, cx, cy, r, 0, 2 * PI);
    cairo_stroke(cr);

    for (int rr = 100; rr <= 220; rr += 60) {
        setc(cr, {0.02, 0.45, 1.0, 0.35});
        cairo_set_line_width(cr, 1.5);
        cairo_arc(cr, cx, cy, rr, 0, 2 * PI);
        cairo_stroke(cr);
    }
    line(cr, cx - r, cy, cx + r, cy, {0.02, 0.45, 1, 0.45}, 1.5);
    line(cr, cx, cy - r, cx, cy + r, {0.02, 0.45, 1, 0.45}, 1.5);

    for (int a = 0; a < 360; a += 5) {
        const double theta = (a - 90) * DEG;
        const double inner = r - ((a % 30 == 0) ? 18 : 10);
        line(cr, cx + inner * std::cos(theta), cy + inner * std::sin(theta),
             cx + r * std::cos(theta), cy + r * std::sin(theta), CYAN, a % 30 == 0 ? 2.5 : 1.2);
        if (a % 30 == 0) {
            const double tx = cx + (r + 36) * std::cos(theta);
            const double ty = cy + (r + 36) * std::sin(theta);
            text(cr, std::to_string(a) + "°", tx, ty, 23, WHITE, "normal", 0.5, 0.5);
        }
    }

    text(cr, "BOW", cx, cy - r - 55, 28, WHITE, "bold", 0.5, 0.5);
    text(cr, "STERN", cx, cy + r + 55, 28, WHITE, "bold", 0.5, 0.5);
    text(cr, "PORT", cx - r - 95, cy, 28, WHITE, "bold", 0.5, 0.5);
    text(cr, "STBD", cx + r + 95, cy, 28, WHITE, "bold", 0.5, 0.5);
    draw_boat_top(cr, cx, cy, 0.95, WHITE);

    // Wave arrow: waves coming FROM relative angle toward vessel. 0 deg = bow, clockwise positive.
    const double theta = (d.wave_rel_deg - 90.0) * DEG;
    const double sx = cx + 210 * std::cos(theta);
    const double sy = cy + 210 * std::sin(theta);
    const double ex = cx + 85 * std::cos(theta);
    const double ey = cy + 85 * std::sin(theta);
    line(cr, sx, sy, ex, ey, CYAN, 10);
    const double ah = 22;
    const double dir = std::atan2(ey - sy, ex - sx);
    setc(cr, CYAN);
    cairo_move_to(cr, ex, ey);
    cairo_line_to(cr, ex - ah * std::cos(dir - 0.45), ey - ah * std::sin(dir - 0.45));
    cairo_line_to(cr, ex - ah * std::cos(dir + 0.45), ey - ah * std::sin(dir + 0.45));
    cairo_close_path(cr);
    cairo_fill(cr);
    draw_wave_icon(cr, sx - 35, sy - 35, 0.55, CYAN);

    fill_round(cr, 20, 825, 960, 140, 18, PANEL2, LINE, 2);
    text(cr, fmt_signed(signed_relative_degrees(d.wave_rel_deg), 0) + "°", 70, 895, 82, CYAN, "bold", 0, 0.5);
    text(cr, "REL", 335, 912, 34, CYAN, "bold", 0, 0.5);
    line(cr, 500, 845, 500, 945, LINE, 2);
    draw_wave_icon(cr, 535, 875, 0.55, CYAN);
    text(cr, "FROM", 650, 872, 32, WHITE, "bold", 0, 0.5);
    text(cr, rel_wave_name(d.wave_rel_deg), 650, 922, 50, CYAN, "bold", 0, 0.5);
}

void draw_rot(cairo_t* cr, const model::InsData& d) {
    draw_title_bar(cr, "RATE OF TURN");
    text(cr, "PORT", 95, 215, 40, RED, "bold", 0, 0.5);
    text(cr, "STBD", 810, 215, 40, GREEN, "bold", 0, 0.5);

    const double cx = 500, cy = 560, r = 365;
    setc(cr, {0.02, 0.17, 0.45, 0.85});
    cairo_arc(cr, cx, cy, r, PI, 2 * PI);
    cairo_line_to(cr, cx, cy);
    cairo_close_path(cr);
    cairo_fill_preserve(cr);
    setc(cr, LINE);
    cairo_set_line_width(cr, 4);
    cairo_stroke(cr);

    for (int v = -60; v <= 60; v += 5) {
        const double angle = (-90 + v * 1.5) * DEG;
        const double inner = r - ((v % 30 == 0) ? 52 : (v % 10 == 0 ? 38 : 25));
        Color cc = WHITE;
        if (v <= -55) cc = RED;
        if (v >= 55) cc = GREEN;
        line(cr, cx + inner * std::cos(angle), cy + inner * std::sin(angle),
             cx + r * std::cos(angle), cy + r * std::sin(angle), cc, v % 10 == 0 ? 3.0 : 1.5);
        if (v % 30 == 0) {
            const double tx = cx + (inner - 40) * std::cos(angle);
            const double ty = cy + (inner - 40) * std::sin(angle);
            text(cr, std::to_string(std::abs(v)), tx, ty, 28, WHITE, "bold", 0.5, 0.5);
        }
    }
    text(cr, "°/min", cx, cy - 235, 34, CYAN, "bold", 0.5, 0.5);

    const double val = clampd(d.rot_deg_min, -60, 60);
    const double angle = (-90 + val * 1.5) * DEG;
    const double nx = cx + (r - 100) * std::cos(angle);
    const double ny = cy + (r - 100) * std::sin(angle);
    line(cr, cx, cy, nx, ny, WHITE, 10);
    setc(cr, {0.08, 0.12, 0.18, 1});
    cairo_arc(cr, cx, cy, 42, 0, 2 * PI);
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
    const double h1 = wrap360(d.heading_deg - d.rot_deg_min / 6.0);
    const double h3 = wrap360(d.heading_deg + d.rot_deg_min / 6.0);
    text(cr, fmt(h1, 0) + "°", 300, 805, 42, CYAN, "bold", 0.5, 0.5);
    text(cr, "-10 sec", 300, 850, 25, CYAN, "bold", 0.5, 0.5);
    text(cr, "→", 430, 805, 40, WHITE, "bold", 0.5, 0.5);
    text(cr, fmt(d.heading_deg, 0) + "°", 560, 805, 42, CYAN, "bold", 0.5, 0.5);
    text(cr, "NOW", 560, 850, 25, CYAN, "bold", 0.5, 0.5);
    text(cr, "→", 690, 805, 40, WHITE, "bold", 0.5, 0.5);
    text(cr, fmt(h3, 0) + "°", 820, 805, 42, CYAN, "bold", 0.5, 0.5);
    text(cr, "+10 sec", 820, 850, 25, CYAN, "bold", 0.5, 0.5);
    draw_check_pill(cr, 345, 915, 310, 50, d.system_status, status_color(d.system_status));
}

void draw_status(cairo_t* cr, const model::InsData& d) {
    fill_round(cr, 20, 20, 960, 945, 18, {0.012, 0.025, 0.045, 0.92}, LINE, 2);
    text(cr, "INS STATUS", 60, 75, 55, WHITE, "bold", 0, 0.5);
    draw_check_pill(cr, 590, 42, 350, 60, "SYSTEM HEALTHY", GREEN);
    draw_status_row(cr, 0, "ATTITUDE", d.attitude_status, status_color(d.attitude_status));
    draw_status_row(cr, 1, "HEAVE", d.heave_status, status_color(d.heave_status));
    draw_status_row(cr, 2, "WAVE DIR", d.wave_status, status_color(d.wave_status));
    draw_status_row(cr, 3, "MAG", d.mag_status, status_color(d.mag_status));
    draw_status_row(cr, 4, "GYRO BIAS", d.gyro_bias_status, status_color(d.gyro_bias_status));
    draw_status_row(cr, 5, "ACC BIAS", d.acc_bias_status, status_color(d.acc_bias_status));
    draw_status_row(cr, 6, "TEMP", fmt(d.temp_c, 1) + " °C", CYAN);
    draw_status_row(cr, 7, "SAMPLE", fmt(d.sample_hz, 0) + " Hz", CYAN);
    draw_status_row(cr, 8, "MAG RATE", fmt(d.mag_rate_hz, 0) + " Hz", CYAN);
}

} // namespace ins_display::render
