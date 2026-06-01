#include "render/Icons.h"

#include "render/DrawUtil.h"
#include "util/MathUtil.h"

#include <cmath>

namespace ins_display::render {

namespace {
void chevron(cairo_t* cr, double x, double y, double s, double rot, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, rot);
    setc(cr, c);
    cairo_move_to(cr, 0, -14 * s);
    cairo_line_to(cr, -10 * s, 10 * s);
    cairo_line_to(cr, 0, 5 * s);
    cairo_line_to(cr, 10 * s, 10 * s);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_restore(cr);
}
} // namespace

void draw_wave_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, 3.2 * scale);
    for (int k = 0; k < 3; ++k) {
        const double yy = y + k * 12 * scale;
        cairo_move_to(cr, x, yy);
        for (int i = 0; i < 3; ++i) {
            cairo_rel_curve_to(cr, 9 * scale, -9 * scale, 18 * scale, -9 * scale, 27 * scale, 0);
            cairo_rel_curve_to(cr, 9 * scale, 9 * scale, 18 * scale, 9 * scale, 27 * scale, 0);
        }
        cairo_stroke(cr);
    }
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
}

void draw_boat_top(cairo_t* cr, double cx, double cy, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, cx, cy);
    cairo_scale(cr, scale, scale);

    // Soft cyan halo, then a clean white hull. This matches the mockup better than the old wireframe boat.
    setc(cr, {CYAN.r, CYAN.g, CYAN.b, 0.12});
    cairo_move_to(cr, 0, -82);
    cairo_curve_to(cr, -48, -40, -44, 48, -28, 75);
    cairo_line_to(cr, 28, 75);
    cairo_curve_to(cr, 44, 48, 48, -40, 0, -82);
    cairo_close_path(cr);
    cairo_fill(cr);

    setc(cr, c);
    cairo_set_line_width(cr, 4.5);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_move_to(cr, 0, -78);
    cairo_curve_to(cr, -42, -42, -42, 40, -26, 70);
    cairo_line_to(cr, 26, 70);
    cairo_curve_to(cr, 42, 40, 42, -42, 0, -78);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    setc(cr, {0.94, 0.97, 1.00, 0.13});
    cairo_fill(cr);

    rounded_rect(cr, -22, -22, 44, 42, 10);
    setc(cr, {0.92, 0.96, 1.0, 0.95});
    cairo_fill_preserve(cr);
    setc(cr, c);
    cairo_set_line_width(cr, 3.2);
    cairo_stroke(cr);

    line(cr, -15, -3, 15, -3, BG, 3.0);
    line(cr, 0, -54, 0, 64, {0.86, 0.94, 1.0, 0.65}, 2.0);
    line(cr, -25, 38, 25, 38, {0.86, 0.94, 1.0, 0.65}, 2.0);
    cairo_restore(cr);
}

void draw_boat_front(cairo_t* cr, double cx, double cy, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, cx, cy);
    cairo_scale(cr, scale, scale);

    setc(cr, {CYAN.r, CYAN.g, CYAN.b, 0.10});
    cairo_move_to(cr, -58, 24);
    cairo_line_to(cr, -38, -34);
    cairo_line_to(cr, 38, -34);
    cairo_line_to(cr, 58, 24);
    cairo_line_to(cr, 28, 56);
    cairo_line_to(cr, -28, 56);
    cairo_close_path(cr);
    cairo_fill(cr);

    setc(cr, c);
    cairo_set_line_width(cr, 4.5);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_move_to(cr, -54, 24);
    cairo_line_to(cr, -36, -30);
    cairo_curve_to(cr, -18, -42, 18, -42, 36, -30);
    cairo_line_to(cr, 54, 24);
    cairo_line_to(cr, 26, 52);
    cairo_line_to(cr, -26, 52);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    setc(cr, {0.94, 0.97, 1.0, 0.95});
    cairo_fill(cr);

    rounded_rect(cr, -26, -16, 52, 27, 8);
    setc(cr, {0.08, 0.16, 0.25, 0.78});
    cairo_fill_preserve(cr);
    setc(cr, c);
    cairo_set_line_width(cr, 2.8);
    cairo_stroke(cr);
    line(cr, 0, -48, 0, 54, {0.0, 0.0, 0.0, 0.45}, 2.4);
    cairo_restore(cr);
}

void draw_pitch_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    setc(cr, c);
    cairo_set_line_width(cr, 3.2);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    // profile boat
    cairo_move_to(cr, -42, 11);
    cairo_line_to(cr, 26, 11);
    cairo_line_to(cr, 42, 25);
    cairo_line_to(cr, -30, 25);
    cairo_close_path(cr);
    cairo_stroke(cr);
    line(cr, -6, 9, -6, -42, c, 3.2);
    chevron(cr, -6, -44, 1.0, 0, c);
    line(cr, 18, -34, 18, 4, {c.r, c.g, c.b, 0.60}, 2.0);
    draw_wave_icon(cr, -47, 41, 0.36, c);
    cairo_restore(cr);
}

void draw_roll_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    cairo_rotate(cr, -16.0 * util::DEG);
    setc(cr, c);
    cairo_set_line_width(cr, 3.4);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_move_to(cr, -34, 8);
    cairo_line_to(cr, 28, 8);
    cairo_line_to(cr, 38, 21);
    cairo_line_to(cr, -25, 21);
    cairo_close_path(cr);
    cairo_stroke(cr);
    cairo_restore(cr);
    draw_wave_icon(cr, x - 48 * scale, y + 36 * scale, 0.36 * scale, c);
    setc(cr, c);
    cairo_set_line_width(cr, 3 * scale);
    cairo_arc(cr, x + 30 * scale, y - 27 * scale, 20 * scale, -2.45, -0.35);
    cairo_stroke(cr);
    chevron(cr, x + 45 * scale, y - 42 * scale, 0.70 * scale, 1.0, c);
}

void draw_attitude_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    setc(cr, c);
    cairo_set_line_width(cr, 3.2);
    cairo_arc(cr, 0, 0, 30, 0, 2 * util::PI);
    cairo_stroke(cr);
    line(cr, -22, 3, 22, -3, c, 3.0);
    line(cr, 0, -22, 0, 22, {c.r, c.g, c.b, 0.65}, 2.0);
    cairo_move_to(cr, -12, 8);
    cairo_line_to(cr, 0, -2);
    cairo_line_to(cr, 12, 8);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void draw_compass_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    setc(cr, c);
    cairo_set_line_width(cr, 3.0);
    cairo_arc(cr, 0, 0, 30, 0, 2 * util::PI);
    cairo_stroke(cr);
    for (int i = 0; i < 8; ++i) {
        const double a = i * util::PI / 4.0;
        line(cr, 23 * std::cos(a), 23 * std::sin(a), 29 * std::cos(a), 29 * std::sin(a), c, 2.0);
    }
    setc(cr, c);
    cairo_move_to(cr, 0, -23);
    cairo_line_to(cr, -9, 9);
    cairo_line_to(cr, 0, 3);
    cairo_line_to(cr, 9, 9);
    cairo_close_path(cr);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void draw_magnet_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    setc(cr, c);
    cairo_set_line_width(cr, 5.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_arc(cr, 0, -2, 24, util::PI, 2 * util::PI);
    cairo_stroke(cr);
    line(cr, -24, -2, -24, 20, c, 5.0);
    line(cr, 24, -2, 24, 20, c, 5.0);
    line(cr, -32, 20, -16, 20, RED, 5.0);
    line(cr, 16, 20, 32, 20, CYAN, 5.0);
    cairo_restore(cr);
}

void draw_gyro_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    setc(cr, c);
    cairo_set_line_width(cr, 3.0);
    cairo_arc(cr, 0, 0, 27, 0.25, 5.7);
    cairo_stroke(cr);
    chevron(cr, 27, -14, 0.75, 0.95, c);
    cairo_arc(cr, 0, 0, 13, 0, 2 * util::PI);
    cairo_stroke(cr);
    line(cr, -34, 0, -16, 0, {c.r, c.g, c.b, 0.55}, 2.2);
    line(cr, 16, 0, 34, 0, {c.r, c.g, c.b, 0.55}, 2.2);
    cairo_restore(cr);
}

void draw_accel_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    setc(cr, c);
    cairo_set_line_width(cr, 3.0);
    line(cr, -26, 0, 26, 0, c, 3.0);
    line(cr, 0, 26, 0, -26, c, 3.0);
    chevron(cr, 29, 0, 0.78, util::PI / 2, c);
    chevron(cr, 0, -29, 0.78, 0, c);
    cairo_arc(cr, 0, 0, 5, 0, 2 * util::PI);
    cairo_fill(cr);
    cairo_restore(cr);
}

void draw_thermo_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    setc(cr, c);
    cairo_set_line_width(cr, 3.4);
    rounded_rect(cr, -7, -31, 14, 43, 7);
    cairo_stroke(cr);
    cairo_arc(cr, 0, 17, 13, 0, 2 * util::PI);
    cairo_stroke_preserve(cr);
    setc(cr, {c.r, c.g, c.b, 0.16});
    cairo_fill(cr);
    line(cr, 0, -22, 0, 12, c, 4.0);
    cairo_restore(cr);
}

void draw_samplerate_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    setc(cr, c);
    cairo_set_line_width(cr, 3.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, -36, 0);
    cairo_rel_line_to(cr, 10, 0);
    cairo_rel_line_to(cr, 8, -18);
    cairo_rel_line_to(cr, 13, 36);
    cairo_rel_line_to(cr, 13, -36);
    cairo_rel_line_to(cr, 8, 18);
    cairo_rel_line_to(cr, 14, 0);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void draw_bars_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, scale, scale);
    setc(cr, c);
    rounded_rect(cr, -26, 10, 11, 19, 3); cairo_fill(cr);
    rounded_rect(cr, -8, -2, 11, 31, 3); cairo_fill(cr);
    rounded_rect(cr, 10, -19, 11, 48, 3); cairo_fill(cr);
    cairo_restore(cr);
}

} // namespace ins_display::render
