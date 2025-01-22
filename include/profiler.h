#pragma once

#include <chrono>

#ifdef MINECLONE_PROFILE
#define PROFILE_FUNCTION() mc::scope_timer_t __func__timer__(__func__)
#define PROFILE_SCOPE(name) mc::scope_timer_t __scope_timer__(#name)
#else
#define PROFILE_FUNCTION()
#define PROFILE_SCOPE(name)
#endif

namespace mc
{
  typedef std::chrono::high_resolution_clock profiler_clock_t;
  typedef std::chrono::high_resolution_clock::duration profiler_duration_t;
  typedef std::chrono::high_resolution_clock::time_point profiler_time_t;

  struct profiler_frame_t 
  {
    const char *name;
    profiler_time_t start, finish;
  };

  struct profiler_t
  {
    std::vector<profiler_frame_t> frames;

    inline void add(const char *name, profiler_time_t start, profiler_time_t finish)
    {
      frames.emplace_back(name, start, finish);
    }

    inline void reset()
    {
      frames.clear();
    }

    inline static profiler_t *get_instance()
    {
      static thread_local profiler_t profiler;
      return &profiler;
    }
  };

  struct scope_timer_t
  {
    const char *name;
    profiler_time_t start;

    inline scope_timer_t(const char *name)
    {
      start = profiler_clock_t::now();
    }

    inline ~scope_timer_t()
    {
      profiler_time_t finish = profiler_clock_t::now();
      profiler_t::get_instance()->add(name, start, finish);
    }
  };
};

