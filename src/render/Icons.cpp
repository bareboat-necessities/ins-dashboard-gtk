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

void draw_boat_top(cairo_t* cr, double cx, double cy, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 4 * scale);
    cairo_move_to(cr, cx, cy - 65 * scale);
    cairo_curve_to(cr, cx - 36 * scale, cy - 35 * scale, cx - 32 * scale, cy + 45 * scale, cx - 22 * scale, cy + 60 * scale);
    cairo_line_to(cr, cx + 22 * scale, cy + 60 * scale);
    cairo_curve_to(cr, cx + 32 * scale, cy + 45 * scale, cx + 36 * scale, cy - 35 * scale, cx, cy - 65 * scale);
    cairo_stroke(cr);
    rounded_rect(cr, cx - 20 * scale, cy - 6 * scale, 40 * scale, 32 * scale, 9 * scale);
    cairo_stroke(cr);
}

void draw_boat_front(cairo_t* cr, double cx, double cy, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 4 * scale);
    cairo_move_to(cr, cx - 50 * scale, cy + 22 * scale);
    cairo_line_to(cr, cx - 35 * scale, cy - 26 * scale);
    cairo_line_to(cr, cx + 35 * scale, cy - 26 * scale);
    cairo_line_to(cr, cx + 50 * scale, cy + 22 * scale);
    cairo_line_to(cr, cx + 26 * scale, cy + 48 * scale);
    cairo_line_to(cr, cx - 26 * scale, cy + 48 * scale);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    setc(cr, {0.94, 0.96, 0.98, 0.9});
    cairo_fill(cr);
    setc(cr, BG);
    cairo_set_line_width(cr, 3 * scale);
    cairo_move_to(cr, cx - 30 * scale, cy + 12 * scale);
    cairo_line_to(cr, cx + 30 * scale, cy + 12 * scale);
    cairo_stroke(cr);
    setc(cr, c);
    cairo_set_line_width(cr, 4 * scale);
    cairo_move_to(cr, cx, cy - 44 * scale);
    cairo_line_to(cr, cx, cy + 50 * scale);
    cairo_stroke(cr);
}

void draw_pitch_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    setc(cr, c);
    cairo_set_line_width(cr, 4 * scale);
    cairo_move_to(cr, x - 35 * scale, y + 12 * scale);
    cairo_line_to(cr, x + 35 * scale, y + 12 * scale);
    cairo_line_to(cr, x + 28 * scale, y + 32 * scale);
    cairo_line_to(cr, x - 28 * scale, y + 32 * scale);
    cairo_close_path(cr);
    cairo_stroke(cr);
    line(cr, x, y + 8 * scale, x, y - 42 * scale, c, 4 * scale);
    cairo_move_to(cr, x, y - 42 * scale);
    cairo_line_to(cr, x - 12 * scale, y - 25 * scale);
    cairo_line_to(cr, x + 12 * scale, y - 25 * scale);
    cairo_close_path(cr);
    cairo_fill(cr);
    draw_wave_icon(cr, x - 45 * scale, y + 48 * scale, 0.35 * scale, c);
}

void draw_roll_icon(cairo_t* cr, double x, double y, double scale, Color c) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, -18.0 * util::DEG);
    setc(cr, c);
    cairo_set_line_width(cr, 4 * scale);
    cairo_rectangle(cr, -28 * scale, -20 * scale, 56 * scale, 35 * scale);
    cairo_stroke(cr);
    cairo_restore(cr);
    draw_wave_icon(cr, x - 48 * scale, y + 34 * scale, 0.35 * scale, c);
    setc(cr, c);
    cairo_set_line_width(cr, 3 * scale);
    cairo_arc(cr, x + 28 * scale, y - 28 * scale, 18 * scale, -2.3, -0.4);
    cairo_stroke(cr);
}

} // namespace ins_display::render
