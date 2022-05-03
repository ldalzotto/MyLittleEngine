#pragma once

#include <cor/assertions.hpp>
#include <cor/types.hpp>
#include <m/vec.hpp>

namespace m {

template <typename T, int R, int C> struct mat;

template <typename T> struct mat<T, 4, 4> {
  vec<T, 4> m_data[4];

  T &at(uimax c, uimax r) { return m_data[c].at(r); }
  const T &at(uimax c, uimax r) const { return m_data[c].at(r); }

  static mat getIdentity();
  static mat getZero() { return {0}; };
};

template <> inline mat<f32, 4, 4> mat<f32, 4, 4>::getIdentity() {
  mat<f32, 4, 4> l_mat = {0};
  l_mat.at(0, 0) = 1;
  l_mat.at(1, 1) = 1;
  l_mat.at(2, 2) = 1;
  l_mat.at(3, 3) = 1;
  return l_mat;
};

template <typename T>
static mat<T, 4, 4> look_at(const vec<T, 3> &p_eye, const vec<T, 3> &p_center,
                            const vec<T, 3> &p_up) {
  const vec<T, 3> f = normalize(p_center - p_eye);
  const vec<T, 3> s = normalize(cross(f, p_up));
  const vec<T, 3> u = cross(s, f);

  mat<T, 4, 4> l_result;
  l_result.at(0, 0) = s.x();
  l_result.at(1, 0) = s.y();
  l_result.at(2, 0) = s.z();
  l_result.at(0, 1) = u.x();
  l_result.at(1, 1) = u.y();
  l_result.at(2, 1) = u.z();
  l_result.at(0, 2) = -f.x();
  l_result.at(1, 2) = -f.y();
  l_result.at(2, 2) = -f.z();
  l_result.at(3, 0) = -dot(s, p_eye);
  l_result.at(3, 1) = -dot(u, p_eye);
  l_result.at(3, 2) = dot(f, p_eye);
  l_result.at(0, 3) = 0;
  l_result.at(1, 3) = 0;
  l_result.at(2, 3) = 0;
  l_result.at(3, 3) = 1;

  return l_result;
};

template <typename T>
static mat<T, 4, 4> perspective(T p_fovy, T p_aspect, T p_zNear, T p_zFar) {
  sys::sassert(std::abs(p_aspect - std::numeric_limits<T>::epsilon()) > T(0));

  T const tanHalfFovy = std::tan(p_fovy / T(2));

  mat<T, 4, 4> l_result;
  l_result.at(0, 0) = T(1) / (p_aspect * tanHalfFovy);
  l_result.at(1, 1) = T(1) / (tanHalfFovy);
  l_result.at(2, 2) = -(p_zFar + p_zNear) / (p_zFar - p_zNear);
  l_result.at(2, 3) = -T(1);
  l_result.at(3, 2) = -(T(2) * p_zFar * p_zNear) / (p_zFar - p_zNear);

  l_result.at(0, 1) = 0;
  l_result.at(0, 2) = 0;
  l_result.at(0, 3) = 0;
  l_result.at(1, 0) = 0;
  l_result.at(1, 2) = 0;
  l_result.at(1, 3) = 0;
  l_result.at(2, 0) = 0;
  l_result.at(2, 1) = 0;
  l_result.at(3, 0) = 0;
  l_result.at(3, 1) = 0;
  l_result.at(3, 3) = 0;
  return l_result;
};

template <typename T>
static mat<T, 4, 4> operator*(const mat<T, 4, 4> &p_left,
                              const mat<T, 4, 4> &p_right) {
  mat<T, 4, 4> l_return;
#define col_row(c, r)                                                          \
  (p_left.at(0, c) * p_right.at(r, 0)) +                                       \
      (p_left.at(1, c) * p_right.at(r, 1)) +                                   \
      (p_left.at(2, c) * p_right.at(r, 2)) +                                   \
      (p_left.at(3, c) * p_right.at(r, 3))

  l_return.at(0, 0) = col_row(0, 0);
  l_return.at(1, 0) = col_row(0, 1);
  l_return.at(2, 0) = col_row(0, 2);
  l_return.at(3, 0) = col_row(0, 3);

  l_return.at(0, 1) = col_row(1, 0);
  l_return.at(1, 1) = col_row(1, 1);
  l_return.at(2, 1) = col_row(1, 2);
  l_return.at(3, 1) = col_row(1, 3);

  l_return.at(0, 2) = col_row(2, 0);
  l_return.at(1, 2) = col_row(2, 1);
  l_return.at(2, 2) = col_row(2, 2);
  l_return.at(3, 2) = col_row(2, 3);

  l_return.at(0, 3) = col_row(3, 0);
  l_return.at(1, 3) = col_row(3, 1);
  l_return.at(2, 3) = col_row(3, 2);
  l_return.at(3, 3) = col_row(3, 3);

  return l_return;

#undef col_row
};

template <typename T>
static vec<T, 4> operator*(const mat<T, 4, 4> &p_left,
                           const vec<T, 4> &p_right) {
  vec<T, 4> l_return;
  l_return.at(0) =
      (p_left.at(0, 0) * p_right.at(0)) + (p_left.at(1, 0) * p_right.at(1)) +
      (p_left.at(2, 0) * p_right.at(2)) + (p_left.at(3, 0) * p_right.at(3));
  l_return.at(1) =
      (p_left.at(0, 1) * p_right.at(0)) + (p_left.at(1, 1) * p_right.at(1)) +
      (p_left.at(2, 1) * p_right.at(2)) + (p_left.at(3, 1) * p_right.at(3));
  l_return.at(2) =
      (p_left.at(0, 2) * p_right.at(0)) + (p_left.at(1, 2) * p_right.at(1)) +
      (p_left.at(2, 2) * p_right.at(2)) + (p_left.at(3, 2) * p_right.at(3));
  l_return.at(3) =
      (p_left.at(0, 3) * p_right.at(0)) + (p_left.at(1, 3) * p_right.at(1)) +
      (p_left.at(2, 3) * p_right.at(2)) + (p_left.at(3, 3) * p_right.at(3));

  return l_return;
};

// TODO -> move to another function ?
template <typename T>
static vec<T, 3> operator*(const mat<T, 4, 4> &p_left,
                           const vec<T, 3> &p_right) {
  vec<T, 4> l_tmp = {p_right.x(), p_right.y(), p_right.z(), 1};

  vec<T, 4> l_return = p_left * l_tmp;
  l_return = l_return / l_return.w();
  return vec<T, 3>::make(l_return);
};

} // namespace m