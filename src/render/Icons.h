#pragma once

#include "render/Theme.h"

#include <cairo.h>

namespace ins_display::render {

void draw_wave_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_boat_top(cairo_t* cr, double cx, double cy, double scale, Color c);
void draw_boat_front(cairo_t* cr, double cx, double cy, double scale, Color c);
void draw_pitch_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_roll_icon(cairo_t* cr, double x, double y, double scale, Color c);

} // namespace ins_display::render
