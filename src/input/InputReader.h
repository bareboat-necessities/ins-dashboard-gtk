#pragma once

#include "input/SourceConfig.h"
#include "model/SharedModel.h"

#include <cstddef>
#include <string>
#include <thread>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>
#endif

namespace ins_display::input {

class InputReader {
public:
    InputReader(SourceConfig cfg, model::SharedModel& model);
    ~InputReader();

    InputReader(const InputReader&) = delete;
    InputReader& operator=(const InputReader&) = delete;

    void start();
    void join();

private:
    SourceConfig cfg_;
    model::SharedModel& model_;
    std::thread th_;

    void run();
    void run_demo();
    void run_tcp_forever();
    void run_serial_forever();
    void feed_bytes(const char* buf, std::size_t n, std::string& acc);

#ifdef _WIN32
    SOCKET connect_tcp_once();
    HANDLE open_serial_once();
#else
    int connect_tcp_once();
    int open_serial_once();
#endif
};

} // namespace ins_display::input
