#pragma once

#include <chrono>

class StopWatch {
  public:
    using clock = std::chrono::steady_clock;

    StopWatch() : start_time(clock::now()) {}

    double seconds() const {
        if (stop_time != clock::time_point{}) {
            return std::chrono::duration<double>{stop_time - start_time}.count();
        } else {
            return std::chrono::duration<double>{clock::now() - start_time}.count();
        }
    }

    void restart() {
        stop_time = clock::time_point{};
        start_time = clock::now();
    }

    void stop() { stop_time = clock::now(); }

  private:
    clock::time_point start_time;
    clock::time_point stop_time{};
};
