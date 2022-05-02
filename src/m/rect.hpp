#pragma once

#include <cor/assertions.hpp>
#include <cor/container.hpp>
#include <m/vec.hpp>

namespace m {

template <typename T> struct rect_min_max {
  m::vec<T, 2> m_min;
  m::vec<T, 2> m_max;

  m::vec<T, 2> &min() { return m_min; };
  m::vec<T, 2> &max() { return m_max; };

  static rect_min_max bounding_box(container::range<m::vec<T, 2>> &p_points) {
    assert_debug(p_points.count() > 0);

    rect_min_max l_rect;
    {
      auto &l_point = p_points.at(0);
      l_rect.m_min = l_point;
      l_rect.m_max = l_point;
    }

    for (auto i = 1; i < p_points.count(); ++i) {
      m::vec<T, 2> &l_point = p_points.at(i);
      if (l_rect.m_min.at(0) > l_point.at(0)) {
        l_rect.m_min.at(0) = l_point.at(0);
      } else if (l_rect.m_max.at(0) < l_point.at(0)) {
        l_rect.m_max.at(0) = l_point.at(0);
      }
      if (l_rect.m_min.at(1) > l_point.at(1)) {
        l_rect.m_min.at(1) = l_point.at(1);
      } else if (l_rect.m_max.at(1) < l_point.at(1)) {
        l_rect.m_max.at(1) = l_point.at(1);
      }
    }

    return l_rect;
  };
};

template <typename T> struct rect_point_extend {
  m::vec<T, 2> m_point;
  m::vec<T, 2> m_extend;

  m::vec<T, 2> &point() { return m_point; };

  m::vec<T, 2> &extend() { return m_extend; };

  static rect_point_extend getZero() {
    rect_point_extend l_rect;
    l_rect.m_point = l_rect.m_point.getZero();
    l_rect.m_extend = l_rect.m_extend.getZero();
    return l_rect;
  };
};

} // namespace m