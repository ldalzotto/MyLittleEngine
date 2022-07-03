#pragma once

#include <sys/clock.hpp>
#include <sys/time.h>

namespace clock_sys {

inline extern clock_time get_current_time_micro() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return clock_time{
      .m_seconds = tv.tv_sec,
      .m_micros = tv.tv_usec
  };
};

}; // namespace clock