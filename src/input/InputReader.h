#pragma once

#include "input/SourceConfig.h"
#include "model/SharedModel.h"

#include <string>
#include <sys/types.h>
#include <thread>

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
    void feed_bytes(const char* buf, ssize_t n, std::string& acc);

    int connect_tcp_once();
    int open_serial_once();
};

} // namespace ins_display::input
