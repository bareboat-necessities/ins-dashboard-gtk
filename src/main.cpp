#include "app/App.h"
#include "app/AppState.h"
#include "input/InputReader.h"
#include "input/SourceUri.h"
#include "model/SharedModel.h"

#include <gtk/gtk.h>

#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace {

void print_usage(const char* argv0) {
    std::cerr << "Usage:\n"
              << "  " << argv0 << " [--source] tcp-nmea0183://host:port\n"
              << "  " << argv0 << " [--source] serial-nmea0183:///dev/ttyUSB0?baud=115200\n"
              << "  " << argv0 << " [--source] serial-nmea0183://COM9?baud=115200\n"
              << "  " << argv0 << " [--source] demo://\n";
}

bool set_source_once(std::string& source, bool& source_set, const std::string& value) {
    if (source_set) {
        std::cerr << "Only one source URI may be specified.\n";
        return false;
    }
    source = value;
    source_set = true;
    return true;
}

} // namespace

int main(int argc, char** argv) {
    std::string source = "demo://";
    bool source_set = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--source") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --source.\n";
                print_usage(argv[0]);
                return 2;
            }
            if (!set_source_once(source, source_set, argv[++i])) {
                print_usage(argv[0]);
                return 2;
            }
        } else if (a.rfind("--source=", 0) == 0) {
            if (!set_source_once(source, source_set, a.substr(std::string("--source=").size()))) {
                print_usage(argv[0]);
                return 2;
            }
        } else if (a == "--help" || a == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (!a.empty() && a.front() == '-') {
            std::cerr << "Unknown option " << a << "\n";
            print_usage(argv[0]);
            return 2;
        } else {
            if (!set_source_once(source, source_set, a)) {
                print_usage(argv[0]);
                return 2;
            }
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
    // GTK treats leftover positional arguments as files and unknown options as fatal.
    // Application-specific CLI arguments are parsed above, so only pass argv[0].
    std::vector<char*> gtk_argv{argv[0], nullptr};
    int gtk_argc = 1;
    const int status = g_application_run(G_APPLICATION(app), gtk_argc, gtk_argv.data());
    g_object_unref(app);

    model.stop.store(true);
    reader.join();
    return status;
}
