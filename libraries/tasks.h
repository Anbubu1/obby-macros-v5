#pragma once

#include <chrono>
#include <thread>

void DelayedCall(double seconds, void(*func)());

inline void Wait(std::chrono::duration<double> seconds) {
    using namespace std::chrono;

    auto target = high_resolution_clock::now() + seconds;

    while (true) {
        auto now = high_resolution_clock::now();
        if (now >= target) break;

        auto remaining = target - now;

        if (remaining > 2ms) {
            std::this_thread::sleep_for(remaining - 1ms);
        } else {
            while (high_resolution_clock::now() < target) {
                std::this_thread::yield();
            }
            break;
        }
    }
}

inline void Wait(const double seconds) {
    Wait(std::chrono::duration<double>(seconds));
}

inline double ShortWait() {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    std::this_thread::sleep_for(milliseconds(1));
    auto end = high_resolution_clock::now();
    return duration<double>(end - start).count();
}