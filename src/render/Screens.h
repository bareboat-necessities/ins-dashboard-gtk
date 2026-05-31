#pragma once

#include "model/InsData.h"

#include <cairo.h>
#include <deque>
#include <utility>

namespace ins_display::render {

using HeaveHistory = std::deque<std::pair<double, double>>; // {seconds, heave_m}

void draw_primary(cairo_t* cr, const model::InsData& d);
void draw_heave(cairo_t* cr, const model::InsData& d, const HeaveHistory& hist, double now_s);
void draw_wave(cairo_t* cr, const model::InsData& d);
void draw_rot(cairo_t* cr, const model::InsData& d);
void draw_status(cairo_t* cr, const model::InsData& d);

} // namespace ins_display::render
