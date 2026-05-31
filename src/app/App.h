#pragma once

#include "app/AppState.h"

#include <gtk/gtk.h>

namespace ins_display::app {

void activate_cb(GtkApplication* gtk_app, gpointer user_data);

} // namespace ins_display::app
