#pragma once

#include <thread>
#include <chrono>

#include <unordered_map>

#include "logger.h"

DECLARE_LOG_CATEGORY(Profiler);

#define PROFILE_SCOPE(name) ::Profiler::ScopedTimer __ScopedTimer##name(#name)
#define PROFILE_FUNCTION() ::Profiler::ScopedTimer __ScopedTimer##__FUNCTION__(__FUNCTION__)

namespace Profiler
{
    typedef std::chrono::high_resolution_clock clock;
    typedef clock::time_point time_point;
    typedef clock::duration duration;

    class ScopedTimer
    {
    public:
        ScopedTimer(const char* name);
        ~ScopedTimer();

    private:
        const char* name;
        time_point start;
    };

    class ProfilerThread
    {
    public:
        ProfilerThread();
        ProfilerThread(ProfilerThread&& other) = delete;
        ProfilerThread(const ProfilerThread& other) = delete;

        ~ProfilerThread();

        ProfilerThread& operator=(ProfilerThread&& other) = delete;
        ProfilerThread& operator=(const ProfilerThread& other) = delete;

        static void AddScopedTime(const char* name, duration time);

    private:
        struct ScopedTime
        {
            duration time;
            int count;
        };

    private:
        void Run();

    private:
        std::thread thread;
        std::atomic<bool> exit;

        static std::unordered_map<std::string, ScopedTime> scoped_times;
        static std::mutex scoped_times_mutex;
    };
};

