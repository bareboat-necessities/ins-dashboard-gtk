#pragma once

namespace ins_display::render {

struct Color {
    double r;
    double g;
    double b;
    double a;
};

inline constexpr Color BG{0.015, 0.025, 0.045, 1.0};
inline constexpr Color PANEL{0.02, 0.055, 0.095, 0.80};
inline constexpr Color PANEL2{0.025, 0.075, 0.13, 0.85};
inline constexpr Color LINE{0.16, 0.22, 0.34, 0.95};
inline constexpr Color WHITE{0.92, 0.94, 0.96, 1.0};
inline constexpr Color MUTED{0.55, 0.64, 0.74, 1.0};
inline constexpr Color CYAN{0.02, 0.76, 1.0, 1.0};
inline constexpr Color BLUE{0.02, 0.36, 0.85, 1.0};
inline constexpr Color GREEN{0.48, 0.94, 0.16, 1.0};
inline constexpr Color YELLOW{1.0, 0.78, 0.04, 1.0};
inline constexpr Color RED{1.0, 0.22, 0.16, 1.0};

} // namespace ins_display::render
