#pragma once

#include <chrono>

namespace Utils
{
    typedef ::std::chrono::high_resolution_clock Clock;
    typedef Clock::duration Duration;
    typedef Clock::time_point TimePoint;

    inline TimePoint Now()
    {
        return Clock::now();
    }

    inline Duration LocalTime()
    {
        static TimePoint start = Now();
        return Now() - start;
    }

    class Timer
    {
    public:
        Timer() = default;
        ~Timer() = default;

        inline void Start() noexcept { running = true; start = Now(); }
        inline void Stop() noexcept { running = false; time += Now() - start; }
        inline void Reset() noexcept { time = Duration(0); running = false; }

        inline Duration GetTime() const noexcept { return time; }

        inline bool IsRunning() const noexcept { return running; }

    private:
        bool running = false;
        TimePoint start;
        Duration time;
    };
};

