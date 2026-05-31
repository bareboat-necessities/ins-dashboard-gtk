#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ins_display::util {

std::string trim(std::string s);
bool starts_with(const std::string& s, const std::string& prefix);
std::vector<std::string> split(const std::string& s, char sep);
std::optional<double> to_double(const std::string& s);
std::optional<int> to_int(const std::string& s);
std::string upper(std::string s);
std::string fmt(double v, int precision);
std::string fmt_signed(double v, int precision);

} // namespace ins_display::util
