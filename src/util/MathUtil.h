#pragma once

#include <algorithm>
#include <cmath>

namespace ins_display::util {

inline constexpr double PI = 3.1415926535897932384626433832795;
inline constexpr double DEG = PI / 180.0;

inline double clampd(double x, double lo, double hi) {
    return std::max(lo, std::min(hi, x));
}

inline double wrap360(double x) {
    while (x < 0.0) x += 360.0;
    while (x >= 360.0) x -= 360.0;
    return x;
}

} // namespace ins_display::util
