#pragma once
#include <cor/assertions.hpp>
#include <m/number.hpp>

struct clock_time {
  using clock_time_t = long long;
  static const clock_time_t MAX_MICRO = 1000000;
  clock_time_t m_seconds;
  clock_time_t m_micros;

  static clock_time make_s_ms(clock_time_t p_s, clock_time_t p_mp) {
    return {.m_seconds = p_s, .m_micros = p_mp * 1000};
  };
  static clock_time make_s_mics(clock_time_t p_s, clock_time_t p_mics) {
    return {.m_seconds = p_s, .m_micros = p_mics};
  };

  ui8 operator<=(const clock_time &p_other) {
    if (m_seconds > p_other.m_seconds) {
      return 0;
    } else if (m_seconds < p_other.m_seconds) {
      return 1;
    } else {
      return m_micros <= p_other.m_micros;
    }
  }
  ui8 operator>=(const clock_time &p_other) {
    if (m_seconds < p_other.m_seconds) {
      return 0;
    } else if (m_seconds > p_other.m_seconds) {
      return 1;
    } else {
      return m_micros >= p_other.m_micros;
    }
  }
  clock_time operator-(const clock_time &p_other) {
    clock_time l_return;
    l_return.m_seconds = m_seconds - p_other.m_seconds;
    i32 l_mics_delta = m_micros - p_other.m_micros;
    if (l_mics_delta < 0) {
      l_return.m_seconds -= 1;
      l_mics_delta = l_mics_delta + MAX_MICRO;
    }
    l_return.m_micros = l_mics_delta;
    return l_return;
  }
  clock_time operator+(const clock_time &p_other) {
    clock_time l_return;
    l_return.m_seconds = m_seconds + p_other.m_seconds;
    i32 l_mics_delta = m_micros + p_other.m_micros;
    if (l_mics_delta >= MAX_MICRO) {
      l_return.m_seconds += 1;
      l_mics_delta = l_mics_delta - MAX_MICRO;
    }
    l_return.m_micros = l_mics_delta;
    return l_return;
  }
  clock_time &operator+=(const clock_time &p_other) {
    *this = (*this) + p_other;
    return *this;
  }
};

namespace clock_sys {

extern clock_time get_current_time_micro();

};

struct clock {
  clock_time m_min_delta_ms;
  clock_time m_current;
  clock_time m_last;

  clock_time m_delta;

  void init(clock_time p_delta) {
    m_current = clock_sys::get_current_time_micro();
    m_last = m_current;

    m_min_delta_ms = p_delta;
  };

  ui8 update() {
    auto l_new = clock_sys::get_current_time_micro();
    assert_debug(m_current <= l_new);
    m_current = l_new;
    auto l_delta = m_current - m_last;
    if (l_delta >= m_min_delta_ms) {
      m_delta = {.m_seconds = 0, .m_micros = 0};
      m_last += m_min_delta_ms;
      m_delta += m_min_delta_ms;
      l_delta = m_current - m_last;
      while (l_delta >= m_min_delta_ms) {
        m_last += m_min_delta_ms;
        m_delta += m_min_delta_ms;
        l_delta = m_current - m_last;
      }
      assert_debug(m_last <= m_current);
      return 1;
    }
    assert_debug(m_last <= m_current);
    return 0;
  };

  fix32 delta() {
    return fix32(m_delta.m_seconds) + (fix32(m_delta.m_micros) / 1000000);
  };
};