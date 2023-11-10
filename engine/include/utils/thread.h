#pragma once

#include <thread>
#include <mutex>

namespace Utils
{
    using Thread = ::std::thread;
    using Mutex = ::std::mutex;

    namespace ThisThread
    {
        using namespace ::std::this_thread;
    };

    template<typename MutexType = Mutex>
    using LockGuard = ::std::lock_guard<MutexType>;
};

