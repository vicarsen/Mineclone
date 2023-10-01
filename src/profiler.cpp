#include "profiler.h"

#include <GLFW/glfw3.h>

DEFINE_LOG_CATEGORY(Profiler, CONSOLE_LOGGER(info));

namespace Profiler
{
    ScopedTimer::ScopedTimer(const char* name) :
        name(name), start(clock::now())
    {
    }

    ScopedTimer::~ScopedTimer()
    {
        time_point end = clock::now();
        ProfilerThread::AddScopedTime(name, end - start);
    }

    std::unordered_map<std::string, ProfilerThread::ScopedTime> ProfilerThread::scoped_times;
    std::mutex ProfilerThread::scoped_times_mutex;

    ProfilerThread::ProfilerThread()
    {
        exit = false;
        thread = std::move(std::thread([&]() { Run(); }));
    }

    ProfilerThread::~ProfilerThread()
    {
        exit = true;
        thread.join();
    }

    void ProfilerThread::AddScopedTime(const char* name, duration time)
    {
        std::lock_guard<std::mutex> guard(scoped_times_mutex);
        scoped_times[name].time += time;
        scoped_times[name].count++;
    }

    void ProfilerThread::Run()
    {
        while(!exit)
        {
            float profiler_thread_start = glfwGetTime();

            {
                std::lock_guard<std::mutex> guard(scoped_times_mutex);
                for(const auto&[name, duration] : scoped_times)
                {
                    INFO(Profiler, "{}: {} s ({} steps)", name, std::chrono::duration_cast<std::chrono::nanoseconds>(duration.time).count() / 1'000'000'000.0f, duration.count);
                }

                scoped_times.clear();
            }

            float profiler_thread_end = glfwGetTime();
            float profiler_thread_time = profiler_thread_end - profiler_thread_start;

            float thread_sleep = (profiler_thread_time < 2.0f ? 2.0f - profiler_thread_time : 0.0f);

            if(thread_sleep != 0.0f)
                std::this_thread::sleep_for(std::chrono::microseconds((long long) (thread_sleep * 1'000'000.0f)));
        }
    }
};

