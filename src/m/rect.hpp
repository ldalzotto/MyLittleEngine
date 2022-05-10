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
  const m::vec<T, 2> &min() const { return m_min; };
  const m::vec<T, 2> &max() const { return m_max; };

  ui8 is_valid() const {
    return m_min.x() <= m_max.x() && m_min.y() <= m_max.y();
  };

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
  const m::vec<T, 2> &point() const { return m_point; };
  const m::vec<T, 2> &extend() const { return m_extend; };

  static rect_point_extend getZero() {
    rect_point_extend l_rect;
    l_rect.m_point = l_rect.m_point.getZero();
    l_rect.m_extend = l_rect.m_extend.getZero();
    return l_rect;
  };

  static rect_point_extend build(const rect_min_max<T> &p_other) {
    rect_point_extend l_rect;
    l_rect.point() = p_other.min();
    l_rect.extend() = p_other.max() - p_other.min();
    return l_rect;
  };
};

template <typename T>
rect_min_max<T> bounding_rect(container::range<m::rect_min_max<T>> &p_rects) {
  assert_debug(p_rects.count() > 0);

  rect_min_max<T> l_rect;
  {
    l_rect.min() = p_rects.at(0).min();
    l_rect.max() = p_rects.at(0).max();
  }

  for (auto i = 1; i < p_rects.count(); ++i) {
    m::rect_min_max<T> &l_input_rect = p_rects.at(i);
    if (l_rect.min().x() > l_input_rect.min().x()) {
      l_rect.min().x() = l_input_rect.min().x();
    }
    if (l_rect.max().x() < l_input_rect.max().x()) {
      l_rect.max().x() = l_input_rect.max().x();
    }
    if (l_rect.min().y() > l_input_rect.min().y()) {
      l_rect.min().y() = l_input_rect.min().y();
    }
    if (l_rect.max().y() < l_input_rect.max().y()) {
      l_rect.max().y() = l_input_rect.max().y();
    }
  }

  return l_rect;
};

template <typename T, typename TT>
m::rect_min_max<T> fit_into(const m::rect_min_max<T> &p_rect,
                            const m::rect_point_extend<TT> &p_into) {
  m::rect_min_max<T> l_rect = p_rect;
  if (l_rect.min().x() < p_into.point().x()) {
    l_rect.min().x() = p_into.point().x();
  }

  if (l_rect.min().x() > (p_into.point().x() + p_into.extend().x())) {
    l_rect.min().x() = (p_into.point().x() + p_into.extend().x());
  }

  if (l_rect.min().y() < p_into.point().y()) {
    l_rect.min().y() = p_into.point().y();
  }

  if (l_rect.min().y() > (p_into.point().y() + p_into.extend().y())) {
    l_rect.min().y() = (p_into.point().y() + p_into.extend().y());
  }

  if (l_rect.max().x() > (p_into.point().x() + p_into.extend().x())) {
    l_rect.max().x() = (p_into.point().x() + p_into.extend().x());
  }

  if (l_rect.max().y() > (p_into.point().y() + p_into.extend().y())) {
    l_rect.max().y() = (p_into.point().y() + p_into.extend().y());
  }
  return l_rect;
};

template <typename T, typename TT>
m::rect_min_max<T> fit_into(const m::rect_min_max<T> &p_rect,
                            const m::rect_min_max<TT> &p_into) {
  return fit_into(p_rect, rect_point_extend<TT>::build(p_into));
};

template <typename T, typename TT>
m::rect_min_max<T> extend(const m::rect_min_max<T> &p_rect,
                          const m::rect_min_max<TT> &p_other) {
  m::rect_min_max<T> l_rect = p_rect;
  if (p_other.min().x() < l_rect.min().x()) {
    l_rect.min().x() = p_other.min().x();
  }

  if (p_other.min().y() < l_rect.min().y()) {
    l_rect.min().y() = p_other.min().y();
  }

  if (p_other.max().x() > l_rect.max().x()) {
    l_rect.max().x() = p_other.max().x();
  }

  if (p_other.max().y() >= l_rect.max().y()) {
    l_rect.max().y() = p_other.max().y();
  }
  return l_rect;
};

} // namespace m