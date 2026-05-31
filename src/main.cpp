#include "app/App.h"
#include "app/AppState.h"
#include "input/InputReader.h"
#include "input/SourceUri.h"
#include "model/SharedModel.h"

#include <gtk/gtk.h>

#include <exception>
#include <iostream>
#include <string>

namespace {

void print_usage(const char* argv0) {
    std::cerr << "Usage:\n"
              << "  " << argv0 << " --source tcp-nmea0183://host:port\n"
              << "  " << argv0 << " --source serial-nmea0183:///dev/ttyUSB0?baud=115200\n"
              << "  " << argv0 << " --source demo://\n";
}

} // namespace

int main(int argc, char** argv) {
    std::string source = "demo://";
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--source" && i + 1 < argc) {
            source = argv[++i];
        } else if (a == "--help" || a == "-h") {
            print_usage(argv[0]);
            return 0;
        }
    }

    ins_display::input::SourceConfig cfg;
    try {
        cfg = ins_display::input::parse_source_uri(source);
    } catch (const std::exception& e) {
        std::cerr << "Source URI error: " << e.what() << "\n";
        print_usage(argv[0]);
        return 2;
    }

    ins_display::model::SharedModel model;
    ins_display::app::AppState app_state;
    app_state.model = &model;
    app_state.cfg = cfg;

    ins_display::input::InputReader reader(cfg, model);
    reader.start();

    GtkApplication* app = gtk_application_new("com.example.InsGtk4Nmea", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(ins_display::app::activate_cb), &app_state);
    const int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    model.stop.store(true);
    reader.join();
    return status;
}
