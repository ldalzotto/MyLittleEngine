#pragma once

#include <cor/types.hpp>
#include <m/rect.hpp>

namespace m {

template <typename T, int N> struct polygon;

template <typename T> struct polygon<T, 3> {
  T m_data[3];

  T &p0() { return m_data[0]; };
  T &p1() { return m_data[1]; };
  T &p2() { return m_data[2]; };
  const T &p0() const { return m_data[0]; };
  const T &p1() const { return m_data[1]; };
  const T &p2() const { return m_data[2]; };
};

template <typename T> struct polygon<T, 2> {
  T m_data[2];

  T &p0() { return m_data[0]; };
  T &p1() { return m_data[1]; };
  const T &p0() const { return m_data[0]; };
  const T &p1() const { return m_data[1]; };
};

template <typename T>
T interpolate(polygon<T, 3> &p_polygon, const polygon<f32, 3> &p_weight) {
  return (p_polygon.p0() * p_weight.p0()) + (p_polygon.p1() * p_weight.p1()) +
         (p_polygon.p2() * p_weight.p2());
};
template <typename T>
T interpolate(polygon<T, 3> &p_polygon, const vec<f32, 3> &p_weight) {
  return (p_polygon.p0() * p_weight.x()) + (p_polygon.p1() * p_weight.y()) +
         (p_polygon.p2() * p_weight.z());
};

template <typename T>
m::rect_min_max<T> bounding_rect(polygon<m::vec<T, 2>, 3> &p_polygon) {
  container::range<m::vec<T, 2>> l_points;
  l_points.m_begin = &p_polygon.p0();
  l_points.m_count = 3;
  return m::rect_min_max<T>::bounding_box(l_points);
};

}; // namespace m