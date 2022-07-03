#pragma once

#include <sys/clock.hpp>

inline static clock_time s_next_clock_time = {.m_seconds = 0, .m_micros = 0};

namespace clock_sys {

inline extern clock_time get_current_time_micro() { return s_next_clock_time; };

}; // namespace clock_sys