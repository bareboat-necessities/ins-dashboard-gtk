#include "app/App.h"

#include "render/DrawUtil.h"
#include "render/Screens.h"

#include <algorithm>
#include <chrono>
#include <gdk/gdkkeysyms.h>
#include <mutex>

namespace ins_display::app {

model::InsData snapshot(AppState* app) {
    std::lock_guard<std::mutex> lock(app->model->mtx);
    return app->model->data;
}

namespace {

void draw_cb(GtkDrawingArea*, cairo_t* cr, int width, int height, gpointer user_data) {
    auto* app = static_cast<AppState*>(user_data);
    const model::InsData d = snapshot(app);

    const double side = std::min(width, height);
    const double ox = (width - side) * 0.5;
    const double oy = (height - side) * 0.5;

    cairo_save(cr);
    render::setc(cr, render::BG);
    cairo_paint(cr);
    cairo_translate(cr, ox, oy);
    cairo_scale(cr, side / 1000.0, side / 1000.0);

    render::fill_round(cr, 10, 10, 980, 980, 18, {0.006, 0.014, 0.026, 1.0}, render::LINE, 2.0);

    const auto now = std::chrono::steady_clock::now();
    const double now_s = std::chrono::duration<double>(now - app->t0).count();

    switch (app->screen) {
        case AppState::Screen::Primary: render::draw_primary(cr, d); break;
        case AppState::Screen::Heave: render::draw_heave(cr, d, app->heave_hist, now_s); break;
        case AppState::Screen::Wave: render::draw_wave(cr, d); break;
        case AppState::Screen::Rot: render::draw_rot(cr, d); break;
        case AppState::Screen::Status: render::draw_status(cr, d); break;
    }

    render::text(cr, "1 HELM   2 HEAVE   3 WAVES   4 ROT   5 STATUS", 500, 984, 14, render::MUTED, "bold", 0.5, 0.5);
    cairo_restore(cr);
}

gboolean tick_cb(gpointer user_data) {
    auto* app = static_cast<AppState*>(user_data);
    const model::InsData d = snapshot(app);
    const auto now = std::chrono::steady_clock::now();
    const double t = std::chrono::duration<double>(now - app->t0).count();
    app->heave_hist.emplace_back(t, d.heave_m);
    while (!app->heave_hist.empty() && t - app->heave_hist.front().first > 20.5) {
        app->heave_hist.pop_front();
    }
    gtk_widget_queue_draw(app->drawing);
    return G_SOURCE_CONTINUE;
}

gboolean key_cb(GtkEventControllerKey*, guint keyval, guint, GdkModifierType, gpointer user_data) {
    auto* app = static_cast<AppState*>(user_data);
    switch (keyval) {
        case GDK_KEY_1: app->screen = AppState::Screen::Primary; break;
        case GDK_KEY_2: app->screen = AppState::Screen::Heave; break;
        case GDK_KEY_3: app->screen = AppState::Screen::Wave; break;
        case GDK_KEY_4: app->screen = AppState::Screen::Rot; break;
        case GDK_KEY_5: app->screen = AppState::Screen::Status; break;
        case GDK_KEY_Right:
        case GDK_KEY_space: {
            const int n = static_cast<int>(app->screen);
            app->screen = static_cast<AppState::Screen>((n + 1) % 5);
            break;
        }
        case GDK_KEY_Left: {
            const int n = static_cast<int>(app->screen);
            app->screen = static_cast<AppState::Screen>((n + 4) % 5);
            break;
        }
        case GDK_KEY_Escape:
        case GDK_KEY_q:
        case GDK_KEY_Q: {
            GtkRoot* root = gtk_widget_get_root(app->drawing);
            if (GTK_IS_WINDOW(root)) gtk_window_close(GTK_WINDOW(root));
            return TRUE;
        }
        default:
            return FALSE;
    }
    gtk_widget_queue_draw(app->drawing);
    return TRUE;
}

} // namespace

void activate_cb(GtkApplication* gtk_app, gpointer user_data) {
    auto* app = static_cast<AppState*>(user_data);

    GtkWidget* win = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(win), "INS GTK4 NMEA Display");
    gtk_window_set_default_size(GTK_WINDOW(win), 900, 900);

    GtkWidget* area = gtk_drawing_area_new();
    app->drawing = area;
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(area), 720);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(area), 720);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), draw_cb, app, nullptr);
    gtk_window_set_child(GTK_WINDOW(win), area);

    GtkEventController* key = gtk_event_controller_key_new();
    g_signal_connect(key, "key-pressed", G_CALLBACK(key_cb), app);
    gtk_widget_add_controller(win, key);

    g_timeout_add(33, tick_cb, app);
    gtk_window_present(GTK_WINDOW(win));
}

} // namespace ins_display::app
