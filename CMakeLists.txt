cmake_minimum_required(VERSION 3.16)
project(ins_gtk4_nmea LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

if(WIN32 AND MINGW)
    add_link_options(-static-libstdc++ -static-libgcc)
endif()

add_executable(ins_gtk4_nmea
    src/main.cpp
    src/app/App.cpp
    src/input/InputReader.cpp
    src/input/SourceUri.cpp
    src/nmea/NmeaParser.cpp
    src/render/DrawUtil.cpp
    src/render/Icons.cpp
    src/render/Screens.cpp
    src/sim/DemoData.cpp
    src/util/StringUtil.cpp
)

target_include_directories(ins_gtk4_nmea PRIVATE
    src
    ${GTK4_INCLUDE_DIRS}
)

target_link_directories(ins_gtk4_nmea PRIVATE ${GTK4_LIBRARY_DIRS})
target_link_libraries(ins_gtk4_nmea PRIVATE ${GTK4_LIBRARIES})

if(WIN32)
    target_link_libraries(ins_gtk4_nmea PRIVATE ws2_32)
endif()
target_compile_options(ins_gtk4_nmea PRIVATE ${GTK4_CFLAGS_OTHER} -Wall -Wextra -Wpedantic)
