#pragma once

#include "model/InsData.h"

#include <atomic>
#include <mutex>

namespace ins_display::model {

struct SharedModel {
    std::mutex mtx;
    InsData data;
    std::atomic<bool> stop{false};
};

} // namespace ins_display::model
