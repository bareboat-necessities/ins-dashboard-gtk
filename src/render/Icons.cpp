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
    cairo_set_line_width(cr, 3.0 * scale);

    cairo_arc(cr, x, y, 30 * scale, 0, 2 * util::PI);
    cairo_stroke(cr);

    draw_wave_icon(cr, x - 20 * scale, y - 6 * scale, 0.28 * scale, c);
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

    // Vertical arrow.
    line(cr, x, y + 28 * scale, x, y - 28 * scale, c, 3.6 * scale);
    cairo_move_to(cr, x, y - 40 * scale);
    cairo_line_to(cr, x - 12 * scale, y - 20 * scale);
    cairo_line_to(cr, x + 12 * scale, y - 20 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);

    draw_wave_icon(cr, x - 40 * scale, y + 10 * scale, 0.34 * scale, c);
}

void draw_heave_status_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    line(cr, x, y - 30 * scale, x, y - 2 * scale, c, 3.2 * scale);
    cairo_move_to(cr, x, y - 42 * scale);
    cairo_line_to(cr, x - 10 * scale, y - 24 * scale);
    cairo_line_to(cr, x + 10 * scale, y - 24 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);
    draw_wave_icon(cr, x - 30 * scale, y - 6 * scale, 0.28 * scale, c);
    line(cr, x, y + 18 * scale, x, y + 28 * scale, c, 2.4 * scale);
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
    const Color fill{0.96, 0.97, 0.99, 1.0};

    // Center mast / reference line.
    line(cr, cx, cy - 58 * scale, cx, cy + 44 * scale, c, 3.2 * scale);

    // Upper cabin.
    setc(cr, c);
    cairo_set_line_width(cr, 4.0 * scale);
    cairo_move_to(cr, cx - 18 * scale, cy - 36 * scale);
    cairo_line_to(cr, cx - 14 * scale, cy - 54 * scale);
    cairo_line_to(cr, cx + 14 * scale, cy - 54 * scale);
    cairo_line_to(cr, cx + 18 * scale, cy - 36 * scale);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    setc(cr, fill);
    cairo_fill(cr);

    // Middle bridge/deck.
    setc(cr, c);
    cairo_set_line_width(cr, 4.0 * scale);
    cairo_move_to(cr, cx - 30 * scale, cy - 8 * scale);
    cairo_line_to(cr, cx - 22 * scale, cy - 32 * scale);
    cairo_line_to(cr, cx + 22 * scale, cy - 32 * scale);
    cairo_line_to(cr, cx + 30 * scale, cy - 8 * scale);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    setc(cr, fill);
    cairo_fill(cr);

    // Lower hull.
    setc(cr, c);
    cairo_set_line_width(cr, 4.2 * scale);
    cairo_move_to(cr, cx - 48 * scale, cy + 2 * scale);
    cairo_line_to(cr, cx - 38 * scale, cy + 36 * scale);
    cairo_line_to(cr, cx + 38 * scale, cy + 36 * scale);
    cairo_line_to(cr, cx + 48 * scale, cy + 2 * scale);
    cairo_line_to(cr, cx + 28 * scale, cy - 8 * scale);
    cairo_line_to(cr, cx - 28 * scale, cy - 8 * scale);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    setc(cr, fill);
    cairo_fill(cr);

    // Deck lines like the original mockup.
    line(cr, cx - 24 * scale, cy - 18 * scale, cx + 24 * scale, cy - 18 * scale, c, 3.0 * scale);
    line(cr, cx - 40 * scale, cy + 1 * scale,  cx + 40 * scale, cy + 1 * scale,  c, 3.0 * scale);
}

void draw_pitch_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 3.5 * scale);

    // Side-view hull.
    cairo_move_to(cr, x - 32 * scale, y + 12 * scale);
    cairo_curve_to(cr, x - 18 * scale, y - 2 * scale, x + 16 * scale, y - 2 * scale, x + 34 * scale, y + 10 * scale);
    cairo_line_to(cr, x + 26 * scale, y + 24 * scale);
    cairo_line_to(cr, x - 24 * scale, y + 24 * scale);
    cairo_close_path(cr);
    cairo_stroke(cr);

    // Mast / pitch arrow.
    line(cr, x, y + 10 * scale, x, y - 40 * scale, c, 3.2 * scale);
    cairo_move_to(cr, x, y - 50 * scale);
    cairo_line_to(cr, x - 10 * scale, y - 32 * scale);
    cairo_line_to(cr, x + 10 * scale, y - 32 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);

    // Keel dot.
    cairo_arc(cr, x, y + 26 * scale, 3.5 * scale, 0, 2 * util::PI);
    cairo_fill(cr);
}

void draw_roll_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, -18.0 * util::DEG);

    setc(cr, c);
    cairo_set_line_width(cr, 3.5 * scale);

    // Tilted small hull.
    cairo_move_to(cr, -28 * scale, 8 * scale);
    cairo_curve_to(cr, -18 * scale, -6 * scale, 18 * scale, -6 * scale, 28 * scale, 8 * scale);
    cairo_line_to(cr, 20 * scale, 22 * scale);
    cairo_line_to(cr, -20 * scale, 22 * scale);
    cairo_close_path(cr);
    cairo_stroke(cr);

    cairo_restore(cr);

    // Waves.
    draw_wave_icon(cr, x - 38 * scale, y + 26 * scale, 0.30 * scale, c);

    // Curved roll arrow.
    setc(cr, c);
    cairo_set_line_width(cr, 3.0 * scale);
    cairo_arc(cr, x + 24 * scale, y - 24 * scale, 15 * scale, -2.35, -0.75);
    cairo_stroke(cr);

    cairo_move_to(cr, x + 34 * scale, y - 30 * scale);
    cairo_line_to(cr, x + 30 * scale, y - 18 * scale);
    cairo_line_to(cr, x + 42 * scale, y - 21 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);
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
    setc(cr, c);
    cairo_set_line_width(cr, 5 * scale);
    cairo_arc(cr, x, y, 22 * scale, util::PI, 2 * util::PI);
    cairo_stroke(cr);
    line(cr, x - 22 * scale, y, x - 22 * scale, y + 18 * scale, c, 5 * scale);
    line(cr, x + 22 * scale, y, x + 22 * scale, y + 18 * scale, c, 5 * scale);
    line(cr, x - 28 * scale, y + 18 * scale, x - 16 * scale, y + 18 * scale, RED, 5 * scale);
    line(cr, x + 16 * scale, y + 18 * scale, x + 28 * scale, y + 18 * scale, CYAN, 5 * scale);
}

void draw_gyro_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 2.8 * scale);
    cairo_arc(cr, x, y + 2 * scale, 22 * scale, 0.15 * util::PI, 0.85 * util::PI);
    cairo_stroke(cr);
    cairo_arc(cr, x, y + 2 * scale, 22 * scale, 1.15 * util::PI, 1.85 * util::PI);
    cairo_stroke(cr);
    cairo_arc(cr, x, y + 2 * scale, 30 * scale, 0.30 * util::PI, 0.70 * util::PI);
    cairo_stroke(cr);
    line(cr, x, y - 30 * scale, x, y + 30 * scale, c, 2.8 * scale);
    cairo_move_to(cr, x - 8 * scale, y + 34 * scale);
    cairo_line_to(cr, x, y + 24 * scale);
    cairo_line_to(cr, x + 8 * scale, y + 34 * scale);
    cairo_stroke(cr);
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
    setc(cr, c);
    cairo_set_line_width(cr, 3.0 * scale);
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
    cairo_arc(cr, x, y, 2.5 * scale, 0, 2 * util::PI);
    cairo_fill(cr);
}

void draw_thermo_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 3.5 * scale);
    cairo_arc(cr, x, y + 14 * scale, 12 * scale, 0, 2 * util::PI);
    cairo_stroke_preserve(cr);
    setc(cr, {c.r, c.g, c.b, 0.15});
    cairo_fill(cr);
    setc(cr, c);
    rounded_rect(cr, x - 7 * scale, y - 28 * scale, 14 * scale, 42 * scale, 7 * scale);
    cairo_stroke(cr);
    line(cr, x, y - 22 * scale, x, y + 8 * scale, c, 4 * scale);
}

void draw_samplerate_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 3.0 * scale);
    cairo_move_to(cr, x - 34 * scale, y + 8 * scale);
    cairo_curve_to(cr, x - 24 * scale, y - 28 * scale, x - 10 * scale, y - 28 * scale, x, y + 8 * scale);
    cairo_curve_to(cr, x + 10 * scale, y + 44 * scale, x + 24 * scale, y + 44 * scale, x + 34 * scale, y + 8 * scale);
    cairo_stroke(cr);
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
