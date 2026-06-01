#include "render/Icons.h"

#include "render/DrawUtil.h"
#include "util/MathUtil.h"

namespace ins_display::render {

void draw_wave_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 3.0 * scale);
    for (int k = 0; k < 3; ++k) {
        const double yy = y + k * 12 * scale;
        cairo_move_to(cr, x, yy);
        for (int i = 0; i < 4; ++i) {
            cairo_rel_curve_to(cr, 7 * scale, -8 * scale, 15 * scale, -8 * scale, 22 * scale, 0);
            cairo_rel_curve_to(cr, 7 * scale, 8 * scale, 15 * scale, 8 * scale, 22 * scale, 0);
        }
        cairo_stroke(cr);
    }
}

void draw_wave_circle_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 3.6 * scale);
    cairo_arc(cr, x, y, 30 * scale, 0, 2 * util::PI);
    cairo_stroke(cr);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    for (int k = 0; k < 3; ++k) {
        const double yy = y - 10 * scale + k * 10 * scale;
        cairo_move_to(cr, x - 20 * scale, yy);
        for (int i = 0; i < 2; ++i) {
            cairo_rel_curve_to(cr, 5 * scale, -5 * scale, 10 * scale, -5 * scale, 15 * scale, 0);
            cairo_rel_curve_to(cr, 5 * scale, 5 * scale, 10 * scale, 5 * scale, 15 * scale, 0);
        }
        cairo_stroke(cr);
    }
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
}

void draw_wave_from_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    draw_wave_icon(cr, x - 34 * scale, y - 26 * scale, 0.40 * scale, c);
    const double x1 = x - 8 * scale;
    const double y1 = y + 8 * scale;
    const double x2 = x - 86 * scale;
    const double y2 = y + 82 * scale;
    glow_line(cr, x1, y1, x2, y2, c, 7.5 * scale, 12.0 * scale);
    const double dir = std::atan2(y2 - y1, x2 - x1);
    const double ah = 18 * scale;
    setc(cr, c);
    cairo_move_to(cr, x2, y2);
    cairo_line_to(cr, x2 - ah * std::cos(dir - 0.48), y2 - ah * std::sin(dir - 0.48));
    cairo_line_to(cr, x2 - ah * std::cos(dir + 0.48), y2 - ah * std::sin(dir + 0.48));
    cairo_close_path(cr);
    cairo_fill(cr);
}

void draw_boat_wave_badge_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    setc(cr, c);
    cairo_set_line_width(cr, 3.0 * scale);
    cairo_move_to(cr, -18 * scale, -2 * scale);
    cairo_curve_to(cr, -13 * scale, -15 * scale, 13 * scale, -15 * scale, 18 * scale, -2 * scale);
    cairo_line_to(cr, 13 * scale, 12 * scale);
    cairo_line_to(cr, -13 * scale, 12 * scale);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    setc(cr, {c.r, c.g, c.b, 0.10});
    cairo_fill(cr);
    setc(cr, c);
    line(cr, -10 * scale, -18 * scale, -8 * scale, -4 * scale, c, 3.0 * scale);
    line(cr, -8 * scale, -18 * scale, 10 * scale, -18 * scale, c, 3.0 * scale);
    line(cr, 10 * scale, -18 * scale, 12 * scale, -4 * scale, c, 3.0 * scale);
    cairo_restore(cr);
    draw_wave_icon(cr, x - 34 * scale, y + 24 * scale, 0.36 * scale, c);
}

void draw_vertical_motion_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    line(cr, x, y - 88 * scale, x, y - 40 * scale, c, 4.0 * scale);
    cairo_move_to(cr, x, y - 102 * scale);
    cairo_line_to(cr, x - 14 * scale, y - 80 * scale);
    cairo_line_to(cr, x + 14 * scale, y - 80 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);
    for (int i = 0; i < 5; ++i) {
        const double yy = y - 64 * scale + i * 10 * scale;
        line(cr, x - 14 * scale, yy, x + 14 * scale, yy, c, 3.0 * scale);
    }
    draw_wave_icon(cr, x - 38 * scale, y - 6 * scale, 0.34 * scale, c);
    for (int i = 0; i < 4; ++i) {
        const double yy = y + 30 * scale + i * 10 * scale;
        line(cr, x - 12 * scale, yy, x + 12 * scale, yy, c, 3.0 * scale);
    }
    line(cr, x, y + 70 * scale, x, y + 104 * scale, c, 4.0 * scale);
    cairo_move_to(cr, x, y + 118 * scale);
    cairo_line_to(cr, x - 14 * scale, y + 96 * scale);
    cairo_line_to(cr, x + 14 * scale, y + 96 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);
}

void draw_heave_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    // Up-arrow over three wave crests, matching the compact status icon in the reference art.
    line(cr, x, y - 28 * scale, x, y + 8 * scale, c, 4.2 * scale);
    cairo_move_to(cr, x, y - 42 * scale);
    cairo_line_to(cr, x - 13 * scale, y - 22 * scale);
    cairo_line_to(cr, x - 4 * scale, y - 24 * scale);
    cairo_line_to(cr, x - 4 * scale, y + 8 * scale);
    cairo_line_to(cr, x + 4 * scale, y + 8 * scale);
    cairo_line_to(cr, x + 4 * scale, y - 24 * scale);
    cairo_line_to(cr, x + 13 * scale, y - 22 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);

    draw_wave_icon(cr, x - 39 * scale, y + 20 * scale, 0.34 * scale, c);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
}

void draw_heave_status_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    setc(cr, c);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    // Compact status version: a clean up-arrow stacked over three wave crests.
    line(cr, x, y - 29 * scale, x, y - 6 * scale, c, 4.0 * scale);
    cairo_move_to(cr, x, y - 42 * scale);
    cairo_line_to(cr, x - 13 * scale, y - 24 * scale);
    cairo_move_to(cr, x, y - 42 * scale);
    cairo_line_to(cr, x + 13 * scale, y - 24 * scale);
    cairo_stroke(cr);

    cairo_set_line_width(cr, 3.7 * scale);
    for (int k = 0; k < 3; ++k) {
        const double yy = y + (2 + k * 13) * scale;
        cairo_move_to(cr, x - 33 * scale, yy);
        cairo_curve_to(cr, x - 24 * scale, yy + 5 * scale, x - 15 * scale, yy + 5 * scale, x - 7 * scale, yy);
        cairo_curve_to(cr, x + 1 * scale, yy - 5 * scale, x + 10 * scale, yy - 5 * scale, x + 18 * scale, yy);
        cairo_curve_to(cr, x + 25 * scale, yy + 5 * scale, x + 32 * scale, yy + 5 * scale, x + 39 * scale, yy);
        cairo_stroke(cr);
    }
    cairo_restore(cr);
}

void draw_boat_top(cairo_t* cr, double cx, double cy, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 4.0 * scale);
    cairo_move_to(cr, cx, cy - 70 * scale);
    cairo_curve_to(cr, cx - 28 * scale, cy - 44 * scale, cx - 31 * scale, cy + 40 * scale, cx - 16 * scale, cy + 60 * scale);
    cairo_line_to(cr, cx + 16 * scale, cy + 60 * scale);
    cairo_curve_to(cr, cx + 31 * scale, cy + 40 * scale, cx + 28 * scale, cy - 44 * scale, cx, cy - 70 * scale);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    setc(cr, {0.97, 0.98, 0.99, 0.14});
    cairo_fill(cr);

    rounded_rect(cr, cx - 18 * scale, cy - 10 * scale, 36 * scale, 72 * scale, 10 * scale);
    cairo_stroke_preserve(cr);
    setc(cr, {0.97, 0.98, 0.99, 0.10});
    cairo_fill(cr);

    setc(cr, BG);
    cairo_rectangle(cr, cx - 16 * scale, cy + 2 * scale, 32 * scale, 16 * scale);
    cairo_fill(cr);

    setc(cr, c);
    line(cr, cx, cy - 64 * scale, cx, cy + 54 * scale, c, 2.0 * scale);
}

void draw_boat_front(cairo_t* cr, double cx, double cy, double scale, Color c) {
    const Color fill{c.r, c.g, c.b, 1.0};
    const Color cutout{0.015, 0.050, 0.100, 1.0};
    const double s = scale;

    cairo_save(cr);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    // A faint halo keeps the reference icon readable on both the blue sky and dark sea.
    setc(cr, {c.r, c.g, c.b, 0.18});
    cairo_set_line_width(cr, 8.0 * s);
    cairo_move_to(cr, cx - 48 * s, cy + 6 * s);
    cairo_line_to(cr, cx - 36 * s, cy + 36 * s);
    cairo_line_to(cr, cx - 22 * s, cy + 48 * s);
    cairo_line_to(cr, cx + 22 * s, cy + 48 * s);
    cairo_line_to(cr, cx + 36 * s, cy + 36 * s);
    cairo_line_to(cr, cx + 48 * s, cy + 6 * s);
    cairo_stroke(cr);

    // Windowed bridge, drawn as white frames so the horizon remains visible through it.
    setc(cr, c);
    cairo_set_line_width(cr, 4.8 * s);
    cairo_move_to(cr, cx - 28 * s, cy + 6 * s);
    cairo_line_to(cr, cx - 21 * s, cy - 28 * s);
    cairo_line_to(cr, cx - 13 * s, cy - 52 * s);
    cairo_curve_to(cr, cx - 8 * s, cy - 56 * s, cx + 8 * s, cy - 56 * s, cx + 13 * s, cy - 52 * s);
    cairo_line_to(cr, cx + 21 * s, cy - 28 * s);
    cairo_line_to(cr, cx + 28 * s, cy + 6 * s);
    cairo_stroke(cr);

    // Bridge tiers and mullions match the upright, front-facing vessel in the reference.
    line(cr, cx - 22 * s, cy - 26 * s, cx + 22 * s, cy - 26 * s, c, 4.0 * s);
    line(cr, cx - 26 * s, cy - 6 * s, cx + 26 * s, cy - 6 * s, c, 4.0 * s);
    line(cr, cx, cy - 56 * s, cx, cy + 8 * s, c, 4.0 * s);
    line(cr, cx - 15 * s, cy - 48 * s, cx - 19 * s, cy - 8 * s, c, 3.2 * s);
    line(cr, cx + 15 * s, cy - 48 * s, cx + 19 * s, cy - 8 * s, c, 3.2 * s);

    // Solid bow/hull with a slight V so it reads as a ship seen head-on, not a flat block.
    setc(cr, fill);
    cairo_move_to(cr, cx - 52 * s, cy + 6 * s);
    cairo_line_to(cr, cx - 36 * s, cy + 40 * s);
    cairo_curve_to(cr, cx - 21 * s, cy + 49 * s, cx + 21 * s, cy + 49 * s, cx + 36 * s, cy + 40 * s);
    cairo_line_to(cr, cx + 52 * s, cy + 6 * s);
    cairo_line_to(cr, cx + 28 * s, cy - 5 * s);
    cairo_line_to(cr, cx, cy + 2 * s);
    cairo_line_to(cr, cx - 28 * s, cy - 5 * s);
    cairo_close_path(cr);
    cairo_fill(cr);

    // Dark bow seams reproduce the cockpit/bow contour visible in the mockup.
    line(cr, cx - 45 * s, cy + 11 * s, cx, cy + 21 * s, cutout, 2.6 * s);
    line(cr, cx + 45 * s, cy + 11 * s, cx, cy + 21 * s, cutout, 2.6 * s);
    line(cr, cx, cy + 2 * s, cx, cy + 44 * s, cutout, 2.4 * s);

    // Re-draw the central reference line on top, like the white centerline in the source art.
    line(cr, cx, cy - 64 * s, cx, cy + 48 * s, c, 3.2 * s);
    line(cr, cx - 36 * s, cy + 6 * s, cx + 36 * s, cy + 6 * s, c, 3.0 * s);

    cairo_restore(cr);
}

void draw_pitch_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    const double s = scale;
    setc(cr, c);
    cairo_set_line_width(cr, 3.8 * s);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    // Side-profile boat with a slightly raised bow, like the pitch tile in the reference.
    cairo_move_to(cr, x - 43 * s, y + 2 * s);
    cairo_line_to(cr, x - 27 * s, y + 25 * s);
    cairo_curve_to(cr, x - 12 * s, y + 32 * s, x + 17 * s, y + 30 * s, x + 34 * s, y + 23 * s);
    cairo_line_to(cr, x + 42 * s, y + 3 * s);
    cairo_curve_to(cr, x + 16 * s, y - 2 * s, x - 15 * s, y - 3 * s, x - 43 * s, y + 2 * s);
    cairo_close_path(cr);
    cairo_stroke(cr);

    line(cr, x - 33 * s, y + 7 * s, x + 36 * s, y + 7 * s, c, 3.0 * s);
    line(cr, x - 8 * s, y + 8 * s, x - 4 * s, y + 27 * s, c, 2.6 * s);
    line(cr, x + 16 * s, y + 8 * s, x + 12 * s, y + 27 * s, c, 2.6 * s);

    // Vertical pitch arrow rising from the deck.
    line(cr, x, y + 4 * s, x, y - 42 * s, c, 3.8 * s);
    cairo_move_to(cr, x, y - 55 * s);
    cairo_line_to(cr, x - 12 * s, y - 34 * s);
    cairo_line_to(cr, x - 4 * s, y - 36 * s);
    cairo_line_to(cr, x - 4 * s, y + 4 * s);
    cairo_line_to(cr, x + 4 * s, y + 4 * s);
    cairo_line_to(cr, x + 4 * s, y - 36 * s);
    cairo_line_to(cr, x + 12 * s, y - 34 * s);
    cairo_close_path(cr);
    cairo_fill(cr);

    draw_wave_icon(cr, x - 45 * s, y + 30 * s, 0.23 * s, c);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
}

void draw_roll_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    const double s = scale;
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, -14.0 * util::DEG);

    setc(cr, c);
    cairo_set_line_width(cr, 3.6 * s);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    // Tilted front-facing vessel with cabin windows, closer to the roll badge in the mockup.
    cairo_move_to(cr, -30 * s, 4 * s);
    cairo_line_to(cr, -22 * s, -17 * s);
    cairo_line_to(cr, -12 * s, -27 * s);
    cairo_line_to(cr, 11 * s, -27 * s);
    cairo_line_to(cr, 22 * s, -16 * s);
    cairo_line_to(cr, 31 * s, 5 * s);
    cairo_stroke(cr);
    line(cr, -22 * s, -8 * s, 22 * s, -8 * s, c, 3.0 * s);
    line(cr, 0, -27 * s, 0, 20 * s, c, 2.8 * s);

    cairo_move_to(cr, -38 * s, 3 * s);
    cairo_line_to(cr, -27 * s, 25 * s);
    cairo_curve_to(cr, -12 * s, 35 * s, 13 * s, 35 * s, 28 * s, 25 * s);
    cairo_line_to(cr, 38 * s, 3 * s);
    cairo_line_to(cr, 17 * s, 9 * s);
    cairo_line_to(cr, 0, 15 * s);
    cairo_line_to(cr, -17 * s, 9 * s);
    cairo_close_path(cr);
    cairo_stroke(cr);
    cairo_restore(cr);

    draw_wave_icon(cr, x - 42 * s, y + 30 * s, 0.32 * s, c);

    // Small curved roll arrow at the upper-right.
    setc(cr, c);
    cairo_set_line_width(cr, 3.2 * s);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_arc(cr, x + 25 * s, y - 30 * s, 16 * s, -2.45, -0.82);
    cairo_stroke(cr);
    cairo_move_to(cr, x + 36 * s, y - 36 * s);
    cairo_line_to(cr, x + 31 * s, y - 22 * s);
    cairo_line_to(cr, x + 45 * s, y - 26 * s);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
}

void draw_attitude_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 3.0 * scale);
    cairo_arc(cr, x, y, 28 * scale, 0, 2 * util::PI);
    cairo_stroke(cr);
    line(cr, x - 34 * scale, y, x + 34 * scale, y, c, 2.2 * scale);
    line(cr, x - 22 * scale, y - 10 * scale, x - 22 * scale, y + 10 * scale, c, 1.7 * scale);
    line(cr, x + 22 * scale, y - 10 * scale, x + 22 * scale, y + 10 * scale, c, 1.7 * scale);
    line(cr, x, y + 16 * scale, x, y + 24 * scale, c, 2.0 * scale);
    cairo_arc(cr, x, y, 5.0 * scale, 0, 2 * util::PI);
    cairo_fill(cr);
    cairo_move_to(cr, x, y - 26 * scale);
    cairo_line_to(cr, x - 5.5 * scale, y - 14 * scale);
    cairo_line_to(cr, x + 5.5 * scale, y - 14 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);
}

void draw_compass_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 3 * scale);
    cairo_arc(cr, x, y, 28 * scale, 0, 2 * util::PI);
    cairo_stroke(cr);
    text(cr, "N", x, y - 18 * scale, 16 * scale, c, "bold", 0.5, 0.5);
    cairo_move_to(cr, x, y - 8 * scale);
    cairo_line_to(cr, x - 11 * scale, y + 18 * scale);
    cairo_line_to(cr, x, y + 10 * scale);
    cairo_line_to(cr, x + 11 * scale, y + 18 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);
}

void draw_magnet_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    setc(cr, c);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(cr, 4.0 * scale);

    // Reference-style horseshoe magnet: one-color outline with squared pole feet.
    cairo_move_to(cr, x - 25 * scale, y + 27 * scale);
    cairo_line_to(cr, x - 25 * scale, y - 9 * scale);
    cairo_curve_to(cr, x - 25 * scale, y - 28 * scale, x - 13 * scale, y - 40 * scale, x, y - 40 * scale);
    cairo_curve_to(cr, x + 13 * scale, y - 40 * scale, x + 25 * scale, y - 28 * scale, x + 25 * scale, y - 9 * scale);
    cairo_line_to(cr, x + 25 * scale, y + 27 * scale);
    cairo_stroke(cr);

    cairo_set_line_width(cr, 3.2 * scale);
    line(cr, x - 25 * scale, y + 12 * scale, x - 10 * scale, y + 12 * scale, c, 3.2 * scale);
    line(cr, x + 10 * scale, y + 12 * scale, x + 25 * scale, y + 12 * scale, c, 3.2 * scale);
    line(cr, x - 34 * scale, y + 27 * scale, x - 16 * scale, y + 27 * scale, c, 4.0 * scale);
    line(cr, x + 16 * scale, y + 27 * scale, x + 34 * scale, y + 27 * scale, c, 4.0 * scale);
    cairo_restore(cr);
}

void draw_gyro_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    setc(cr, c);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    // Reference-style gyro bias mark: vertical axis through a tilted equatorial ring.
    line(cr, x, y - 36 * scale, x, y + 28 * scale, c, 3.6 * scale);

    cairo_set_line_width(cr, 3.2 * scale);
    cairo_move_to(cr, x - 37 * scale, y + 7 * scale);
    cairo_curve_to(cr, x - 31 * scale, y + 23 * scale, x + 31 * scale, y + 23 * scale, x + 37 * scale, y + 7 * scale);
    cairo_curve_to(cr, x + 31 * scale, y - 6 * scale, x - 31 * scale, y - 6 * scale, x - 37 * scale, y + 7 * scale);
    cairo_stroke(cr);

    cairo_set_line_width(cr, 2.5 * scale);
    cairo_arc(cr, x, y + 7 * scale, 24 * scale, 0.10 * util::PI, 0.90 * util::PI);
    cairo_stroke(cr);
    cairo_arc(cr, x, y + 7 * scale, 24 * scale, 1.10 * util::PI, 1.90 * util::PI);
    cairo_stroke(cr);

    line(cr, x, y + 32 * scale, x, y + 40 * scale, c, 3.0 * scale);
    line(cr, x - 20 * scale, y + 29 * scale, x - 13 * scale, y + 37 * scale, c, 3.0 * scale);
    line(cr, x + 20 * scale, y + 29 * scale, x + 13 * scale, y + 37 * scale, c, 3.0 * scale);
    cairo_restore(cr);
}

void draw_accel_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_arc(cr, x, y, 5.5 * scale, 0, 2 * util::PI);
    cairo_fill(cr);
    line(cr, x, y, x, y - 26 * scale, c, 2.8 * scale);
    line(cr, x, y, x - 23 * scale, y + 16 * scale, c, 2.8 * scale);
    line(cr, x, y, x + 23 * scale, y + 16 * scale, c, 2.8 * scale);
    auto arrow = [&](double ex, double ey, double angle) {
        cairo_move_to(cr, ex, ey);
        cairo_line_to(cr, ex - 8 * scale * std::cos(angle - 0.6), ey - 8 * scale * std::sin(angle - 0.6));
        cairo_line_to(cr, ex - 8 * scale * std::cos(angle + 0.6), ey - 8 * scale * std::sin(angle + 0.6));
        cairo_close_path(cr);
        cairo_fill(cr);
    };
    arrow(x, y - 26 * scale, -util::PI / 2.0);
    arrow(x - 23 * scale, y + 16 * scale, util::PI - 0.60);
    arrow(x + 23 * scale, y + 16 * scale, 0.60);
}

void draw_clock_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    setc(cr, c);
    cairo_set_line_width(cr, 3.0 * scale);

    // Start clock arcs as independent paths. cairo_arc() connects to any
    // existing current point, which can leave a straight-line artifact from
    // the previously drawn clock hand into the center dot on the Heave screen.
    cairo_new_path(cr);
    cairo_arc(cr, x, y, 28 * scale, 0, 2 * util::PI);
    cairo_stroke(cr);
    for (int i = 0; i < 12; ++i) {
        const double a = i * util::PI / 6.0 - util::PI / 2.0;
        const double r1 = (i % 3 == 0) ? 18 * scale : 22 * scale;
        const double r2 = 24 * scale;
        line(cr, x + r1 * std::cos(a), y + r1 * std::sin(a), x + r2 * std::cos(a), y + r2 * std::sin(a), c, 2.0 * scale);
    }
    line(cr, x, y, x, y - 14 * scale, c, 3.0 * scale);
    line(cr, x, y, x + 10 * scale, y - 2 * scale, c, 3.0 * scale);

    cairo_new_path(cr);
    cairo_arc(cr, x, y, 2.5 * scale, 0, 2 * util::PI);
    cairo_fill(cr);
    cairo_restore(cr);
}

void draw_thermo_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    setc(cr, c);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(cr, 3.6 * scale);

    // Tall thermometer outline matching the slim reference glyph.
    cairo_move_to(cr, x - 7 * scale, y + 4 * scale);
    cairo_line_to(cr, x - 7 * scale, y - 32 * scale);
    cairo_curve_to(cr, x - 7 * scale, y - 40 * scale, x + 7 * scale, y - 40 * scale, x + 7 * scale, y - 32 * scale);
    cairo_line_to(cr, x + 7 * scale, y + 4 * scale);
    cairo_stroke(cr);

    line(cr, x, y - 26 * scale, x, y + 10 * scale, c, 4.2 * scale);
    cairo_arc(cr, x, y + 17 * scale, 12 * scale, 0, 2 * util::PI);
    cairo_stroke_preserve(cr);
    setc(cr, {c.r, c.g, c.b, 0.18});
    cairo_fill(cr);
    cairo_restore(cr);
}

void draw_samplerate_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    setc(cr, c);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(cr, 4.0 * scale);

    // Smooth oscillating sample waveform like the reference status row.
    cairo_move_to(cr, x - 34 * scale, y + 14 * scale);
    cairo_curve_to(cr, x - 27 * scale, y - 27 * scale, x - 15 * scale, y - 27 * scale, x - 7 * scale, y + 14 * scale);
    cairo_curve_to(cr, x - 1 * scale, y + 47 * scale, x + 12 * scale, y + 47 * scale, x + 18 * scale, y + 14 * scale);
    cairo_curve_to(cr, x + 22 * scale, y - 4 * scale, x + 29 * scale, y - 3 * scale, x + 36 * scale, y + 4 * scale);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void draw_bars_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    line(cr, x - 28 * scale, y + 24 * scale, x + 28 * scale, y + 24 * scale, c, 2.4 * scale);
    cairo_rectangle(cr, x - 22 * scale, y + 6 * scale, 5 * scale, 12 * scale);
    cairo_rectangle(cr, x - 10 * scale, y - 2 * scale, 5 * scale, 20 * scale);
    cairo_rectangle(cr, x + 2 * scale, y - 12 * scale, 5 * scale, 30 * scale);
    cairo_rectangle(cr, x + 14 * scale, y - 24 * scale, 5 * scale, 42 * scale);
    cairo_fill(cr);
}

} // namespace ins_display::render
