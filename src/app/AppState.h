#pragma once

#include "input/SourceConfig.h"
#include "model/SharedModel.h"
#include "render/Screens.h"

#include <chrono>
#include <gtk/gtk.h>

namespace ins_display::app {

struct AppState {
    enum class Screen { Primary, Heave, Wave, Rot, Status };

    model::SharedModel* model = nullptr;
    GtkWidget* drawing = nullptr;
    input::SourceConfig cfg;
    Screen screen = Screen::Primary;
    render::HeaveHistory heave_hist;
    std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
};

model::InsData snapshot(AppState* app);

} // namespace ins_display::app
