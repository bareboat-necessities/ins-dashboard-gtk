#include "render/DrawUtil.h"

#include "util/MathUtil.h"
#include "util/StringUtil.h"

#include <algorithm>
#include <cmath>

namespace ins_display::render {

void setc(cairo_t* cr, Color c) {
    cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a);
}

void rounded_rect(cairo_t* cr, double x, double y, double w, double h, double r) {
    const double rr = std::min(r, std::min(w, h) / 2.0);
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - rr, y + rr, rr, -util::PI / 2, 0);
    cairo_arc(cr, x + w - rr, y + h - rr, rr, 0, util::PI / 2);
    cairo_arc(cr, x + rr, y + h - rr, rr, util::PI / 2, util::PI);
    cairo_arc(cr, x + rr, y + rr, rr, util::PI, 3 * util::PI / 2);
    cairo_close_path(cr);
}

void fill_round(cairo_t* cr, double x, double y, double w, double h, double r, Color fill, Color stroke, double lw) {
    rounded_rect(cr, x, y, w, h, r);
    setc(cr, fill);
    cairo_fill_preserve(cr);
    if (lw > 0.0) {
        setc(cr, stroke);
        cairo_set_line_width(cr, lw);
        cairo_stroke(cr);
    } else {
        cairo_new_path(cr);
    }
}

void fill_round_gradient(cairo_t* cr, double x, double y, double w, double h, double r,
                         Color top, Color bottom, Color stroke, double lw) {
    rounded_rect(cr, x, y, w, h, r);
    cairo_pattern_t* pat = cairo_pattern_create_linear(x, y, x, y + h);
    cairo_pattern_add_color_stop_rgba(pat, 0.0, top.r, top.g, top.b, top.a);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, bottom.r, bottom.g, bottom.b, bottom.a);
    cairo_set_source(cr, pat);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(pat);
    if (lw > 0.0) {
        setc(cr, stroke);
        cairo_set_line_width(cr, lw);
        cairo_stroke(cr);
    } else {
        cairo_new_path(cr);
    }
}

void fill_circle_gradient(cairo_t* cr, double cx, double cy, double r, Color inner, Color outer,
                          Color stroke, double lw) {
    cairo_new_path(cr);
    cairo_arc(cr, cx, cy, r, 0, 2 * util::PI);
    cairo_pattern_t* pat = cairo_pattern_create_radial(cx, cy, 0.0, cx, cy, r);
    cairo_pattern_add_color_stop_rgba(pat, 0.0, inner.r, inner.g, inner.b, inner.a);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, outer.r, outer.g, outer.b, outer.a);
    cairo_set_source(cr, pat);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(pat);
    if (lw > 0.0) {
        setc(cr, stroke);
        cairo_set_line_width(cr, lw);
        cairo_stroke(cr);
    } else {
        cairo_new_path(cr);
    }
}

void line(cairo_t* cr, double x1, double y1, double x2, double y2, Color c, double lw) {
    setc(cr, c);
    cairo_set_line_width(cr, lw);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
}

void glow_line(cairo_t* cr, double x1, double y1, double x2, double y2, Color c, double lw, double glow) {
    for (int i = 3; i >= 1; --i) {
        const double extra = glow * i / 3.0;
        setc(cr, {c.r, c.g, c.b, 0.08 * i});
        cairo_set_line_width(cr, lw + extra);
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
    }
    line(cr, x1, y1, x2, y2, c, lw);
}

void dashed_circle(cairo_t* cr, double cx, double cy, double r, Color c, double lw, double dash_on, double dash_off) {
    cairo_new_path(cr);
    setc(cr, c);
    cairo_set_line_width(cr, lw);
    const double dashes[2] = {dash_on, dash_off};
    cairo_set_dash(cr, dashes, 2, 0.0);
    cairo_arc(cr, cx, cy, r, 0, 2 * util::PI);
    cairo_stroke(cr);
    cairo_set_dash(cr, nullptr, 0, 0.0);
}

void text(cairo_t* cr, const std::string& s, double x, double y, double size, Color c,
          const char* weight, double ax, double ay) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           std::string(weight) == "bold" ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, size);
    cairo_text_extents_t ext{};
    cairo_text_extents(cr, s.c_str(), &ext);
    const double px = x - (ext.width * ax + ext.x_bearing);
    const double py = y - (ext.height * ay + ext.y_bearing);
    setc(cr, c);
    cairo_move_to(cr, px, py);
    cairo_show_text(cr, s.c_str());
}

void text_shadow(cairo_t* cr, const std::string& s, double x, double y, double size, Color c,
                 const char* weight, double ax, double ay, Color shadow, double dx, double dy) {
    text(cr, s, x + dx, y + dy, size, shadow, weight, ax, ay);
    text(cr, s, x, y, size, c, weight, ax, ay);
}

void draw_check_pill(cairo_t* cr, double x, double y, double w, double h, const std::string& s, Color c) {
    fill_round_gradient(cr, x, y, w, h, h / 2.0, {0.05, 0.12, 0.08, 0.76}, {0.03, 0.08, 0.05, 0.76}, c, 2.0);
    const double cx = x + h * 0.50;
    const double cy = y + h * 0.50;
    cairo_new_path(cr);
    setc(cr, c);
    cairo_arc(cr, cx, cy, h * 0.25, 0, 2 * util::PI);
    cairo_fill(cr);
    line(cr, cx - h * 0.11, cy, cx - h * 0.03, cy + h * 0.08, BG, 4);
    line(cr, cx - h * 0.03, cy + h * 0.08, cx + h * 0.13, cy - h * 0.11, BG, 4);
    text_shadow(cr, s, x + h * 0.90, y + h * 0.53, h * 0.34, c, "bold", 0.0, 0.5,
                {0.0, 0.0, 0.0, 0.40}, 1.5, 1.5);
}

Color status_color(const std::string& s) {
    const std::string u = util::upper(s);
    if (u.find("UNLOCK") != std::string::npos || u.find("BAD") != std::string::npos || u == "OFF" || u == "N") return RED;
    if (u.find("GOOD") != std::string::npos ||
        u.find("LOCK") != std::string::npos ||
        u.find("STABLE") != std::string::npos ||
        u.find("HEALTH") != std::string::npos ||
        u == "OK" || u == "ON" || u == "Y") return GREEN;
    if (u.find("FAIR") != std::string::npos ||
        u.find("LEARN") != std::string::npos ||
        u.find("ESTIM") != std::string::npos ||
        u.find("WARN") != std::string::npos) return YELLOW;
    return RED;
}

std::string rel_wave_name(double deg) {
    deg = util::wrap360(deg);
    static const char* names[] = {"BOW", "STBD BOW", "STBD BEAM", "STBD QTR", "STERN", "PORT QTR", "PORT BEAM", "PORT BOW"};
    const int idx = static_cast<int>(std::floor((deg + 22.5) / 45.0)) % 8;
    return names[idx];
}

} // namespace ins_display::render
