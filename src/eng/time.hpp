#pragma once

#include <m/number.hpp>

namespace eng {

struct time {
  fix32 m_delta;
  fix32 m_elapsed;

  void allocate() {
    m_delta = 0;
    m_elapsed = 0;
  };

  void increment(fix32 p_delta) {
    m_delta = p_delta;
    m_elapsed += p_delta;
  };
};

} // namespace eng