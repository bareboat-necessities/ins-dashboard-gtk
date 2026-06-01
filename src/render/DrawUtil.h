#pragma once

#include "render/Theme.h"

#include <cairo.h>
#include <string>

namespace ins_display::render {

void setc(cairo_t* cr, Color c);
void rounded_rect(cairo_t* cr, double x, double y, double w, double h, double r);
void fill_round(cairo_t* cr, double x, double y, double w, double h, double r, Color fill, Color stroke = LINE, double lw = 2.0);
void fill_round_gradient(cairo_t* cr, double x, double y, double w, double h, double r,
                         Color top, Color bottom, Color stroke = LINE, double lw = 2.0);
void fill_circle_gradient(cairo_t* cr, double cx, double cy, double r, Color inner, Color outer,
                          Color stroke = LINE, double lw = 0.0);
void line(cairo_t* cr, double x1, double y1, double x2, double y2, Color c, double lw = 2.0);
void glow_line(cairo_t* cr, double x1, double y1, double x2, double y2, Color c, double lw = 2.0, double glow = 10.0);
void dashed_circle(cairo_t* cr, double cx, double cy, double r, Color c, double lw = 1.5,
                   double dash_on = 6.0, double dash_off = 6.0);
void text(cairo_t* cr, const std::string& s, double x, double y, double size, Color c,
          const char* weight = "normal", double ax = 0.0, double ay = 0.0);
void text_shadow(cairo_t* cr, const std::string& s, double x, double y, double size, Color c,
                 const char* weight = "normal", double ax = 0.0, double ay = 0.0,
                 Color shadow = {0.0, 0.0, 0.0, 0.55}, double dx = 2.0, double dy = 2.0);
void draw_check_pill(cairo_t* cr, double x, double y, double w, double h, const std::string& s, Color c = GREEN);
Color status_color(const std::string& s);
std::string rel_wave_name(double deg);

} // namespace ins_display::render
