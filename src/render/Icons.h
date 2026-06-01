#pragma once

#include "render/Theme.h"

#include <cairo.h>

namespace ins_display::render {

void draw_wave_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_wave_circle_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_wave_from_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_boat_wave_badge_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_vertical_motion_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_heave_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_heave_status_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_boat_top(cairo_t* cr, double cx, double cy, double scale, Color c);
void draw_boat_front(cairo_t* cr, double cx, double cy, double scale, Color c);
void draw_pitch_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_roll_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_attitude_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_compass_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_magnet_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_gyro_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_accel_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_clock_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_thermo_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_samplerate_icon(cairo_t* cr, double x, double y, double scale, Color c);
void draw_bars_icon(cairo_t* cr, double x, double y, double scale, Color c);

} // namespace ins_display::render
