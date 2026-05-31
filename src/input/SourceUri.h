#pragma once

#include "input/SourceConfig.h"

#include <string>

namespace ins_display::input {

SourceConfig parse_source_uri(const std::string& uri);

} // namespace ins_display::input
