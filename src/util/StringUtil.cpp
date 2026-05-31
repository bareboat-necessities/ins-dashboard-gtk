#include "util/StringUtil.h"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace ins_display::util {

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    return s;
}

bool starts_with(const std::string& s, const std::string& prefix) {
    return s.rfind(prefix, 0) == 0;
}

std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::string cur;
    std::istringstream iss(s);
    while (std::getline(iss, cur, sep)) out.push_back(cur);
    if (!s.empty() && s.back() == sep) out.emplace_back();
    return out;
}

std::optional<double> to_double(const std::string& s) {
    try {
        size_t idx = 0;
        double v = std::stod(s, &idx);
        while (idx < s.size() && std::isspace(static_cast<unsigned char>(s[idx]))) idx++;
        if (idx != s.size()) return std::nullopt;
        return v;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<int> to_int(const std::string& s) {
    try {
        size_t idx = 0;
        int v = std::stoi(s, &idx, 10);
        while (idx < s.size() && std::isspace(static_cast<unsigned char>(s[idx]))) idx++;
        if (idx != s.size()) return std::nullopt;
        return v;
    } catch (...) {
        return std::nullopt;
    }
}

std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

std::string fmt(double v, int precision) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(precision) << v;
    return os.str();
}

std::string fmt_signed(double v, int precision) {
    std::ostringstream os;
    os << (v >= 0 ? "+" : "") << std::fixed << std::setprecision(precision) << v;
    return os.str();
}

} // namespace ins_display::util
