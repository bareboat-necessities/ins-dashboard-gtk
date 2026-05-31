cmake_minimum_required(VERSION 3.16)
project(ins_gtk4_nmea LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

add_executable(ins_gtk4_nmea
    src/main.cpp
)

target_include_directories(ins_gtk4_nmea PRIVATE ${GTK4_INCLUDE_DIRS})
target_link_directories(ins_gtk4_nmea PRIVATE ${GTK4_LIBRARY_DIRS})
target_link_libraries(ins_gtk4_nmea PRIVATE ${GTK4_LIBRARIES})
target_compile_options(ins_gtk4_nmea PRIVATE ${GTK4_CFLAGS_OTHER} -Wall -Wextra -Wpedantic)

if(UNIX AND NOT APPLE)
    target_link_libraries(ins_gtk4_nmea PRIVATE pthread)
endif()
