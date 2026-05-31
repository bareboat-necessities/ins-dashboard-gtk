#pragma once

#include <string>

namespace ins_display::input {

struct SourceConfig {
    enum class Kind { Demo, Tcp, Serial } kind = Kind::Demo;
    std::string uri = "demo://";
    std::string host;
    int port = 0;
    std::string path;
    int baud = 115200;
};

} // namespace ins_display::input
