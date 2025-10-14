#include <tasks.hpp>

#include <thread>

void DelayedCall(const double seconds, void(*func)()) {
    std::thread([seconds, func]() {
        using namespace std::chrono;
        auto target = high_resolution_clock::now() + duration<double>(seconds);

        std::this_thread::sleep_for(duration<double>(seconds * 0.9));

        while (high_resolution_clock::now() < target) {
            std::this_thread::yield();
        }

        func();
    }).detach();
}