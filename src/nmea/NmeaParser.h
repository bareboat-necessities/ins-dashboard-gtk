#pragma once

#include "model/SharedModel.h"

#include <string>

namespace ins_display::nmea {

bool verify_nmea_checksum(const std::string& raw);
std::string sentence_body_no_checksum(const std::string& raw);
void parse_nmea_line(const std::string& raw, model::SharedModel& model);

} // namespace ins_display::nmea
