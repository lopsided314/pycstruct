#pragma once

#include <chrono>

struct StopWatch {
    using clock = std::chrono::steady_clock;

    StopWatch() : start_time(clock::now()) {}

    double seconds() const {
        return std::chrono::duration<double>{clock::now() - start_time}.count();
    }

    void restart() { start_time = clock::now(); }

    clock::time_point start_time;
};
