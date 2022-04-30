#pragma once

#include <cor/assertions.hpp>
#include <cor/types.hpp>
#include <m/vec.hpp>

namespace m {

template <typename T, int R, int C> struct mat {
  using value_type = T;
  static constexpr int row_count = R;
  static constexpr int col_count = C;

  T m_data[R * C];

  template <int RR, int CC> T &at() { return m_data[(CC * R) + C]; }
  T &at(uimax r, uimax c) { return m_data[(c * R) + r]; }
  const T &at(uimax r, uimax c) const { return m_data[(c * R) + r]; }

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
static mat<T, 4, 4> operator*(const mat<T, 4, 4> &p_left,
                              const mat<T, 4, 4> &p_right) {
  mat<T, 4, 4> l_return;
  l_return.at(0, 0) = (p_left.at(0, 0) * p_right.at(0, 0)) +
                      (p_left.at(0, 1) * p_right.at(1, 0)) +
                      (p_left.at(0, 2) * p_right.at(2, 0)) +
                      (p_left.at(0, 3) * p_right.at(3, 0));
  l_return.at(1, 0) = (p_left.at(1, 0) * p_right.at(0, 0)) +
                      (p_left.at(1, 1) * p_right.at(1, 0)) +
                      (p_left.at(1, 2) * p_right.at(2, 0)) +
                      (p_left.at(1, 3) * p_right.at(3, 0));
  l_return.at(2, 0) = (p_left.at(2, 0) * p_right.at(0, 0)) +
                      (p_left.at(2, 1) * p_right.at(1, 0)) +
                      (p_left.at(2, 2) * p_right.at(2, 0)) +
                      (p_left.at(2, 3) * p_right.at(3, 0));
  l_return.at(3, 0) = (p_left.at(3, 0) * p_right.at(0, 0)) +
                      (p_left.at(3, 1) * p_right.at(1, 0)) +
                      (p_left.at(3, 2) * p_right.at(2, 0)) +
                      (p_left.at(3, 3) * p_right.at(3, 0));

  l_return.at(0, 1) = (p_left.at(0, 0) * p_right.at(0, 1)) +
                      (p_left.at(0, 1) * p_right.at(1, 1)) +
                      (p_left.at(0, 2) * p_right.at(2, 1)) +
                      (p_left.at(0, 3) * p_right.at(3, 1));
  l_return.at(1, 1) = (p_left.at(1, 0) * p_right.at(0, 1)) +
                      (p_left.at(1, 1) * p_right.at(1, 1)) +
                      (p_left.at(1, 2) * p_right.at(2, 1)) +
                      (p_left.at(1, 3) * p_right.at(3, 1));
  l_return.at(2, 1) = (p_left.at(2, 0) * p_right.at(0, 1)) +
                      (p_left.at(2, 1) * p_right.at(1, 1)) +
                      (p_left.at(2, 2) * p_right.at(2, 1)) +
                      (p_left.at(2, 3) * p_right.at(3, 1));
  l_return.at(3, 1) = (p_left.at(3, 0) * p_right.at(0, 1)) +
                      (p_left.at(3, 1) * p_right.at(1, 1)) +
                      (p_left.at(3, 2) * p_right.at(2, 1)) +
                      (p_left.at(3, 3) * p_right.at(3, 1));

  l_return.at(0, 2) = (p_left.at(0, 0) * p_right.at(0, 2)) +
                      (p_left.at(0, 1) * p_right.at(1, 2)) +
                      (p_left.at(0, 2) * p_right.at(2, 2)) +
                      (p_left.at(0, 3) * p_right.at(3, 2));
  l_return.at(1, 2) = (p_left.at(1, 0) * p_right.at(0, 2)) +
                      (p_left.at(1, 1) * p_right.at(1, 2)) +
                      (p_left.at(1, 2) * p_right.at(2, 2)) +
                      (p_left.at(1, 3) * p_right.at(3, 2));
  l_return.at(2, 2) = (p_left.at(2, 0) * p_right.at(0, 2)) +
                      (p_left.at(2, 1) * p_right.at(1, 2)) +
                      (p_left.at(2, 2) * p_right.at(2, 2)) +
                      (p_left.at(2, 3) * p_right.at(3, 2));
  l_return.at(3, 2) = (p_left.at(3, 0) * p_right.at(0, 2)) +
                      (p_left.at(3, 1) * p_right.at(1, 2)) +
                      (p_left.at(3, 2) * p_right.at(2, 2)) +
                      (p_left.at(3, 3) * p_right.at(3, 2));

  l_return.at(0, 3) = (p_left.at(0, 0) * p_right.at(0, 3)) +
                      (p_left.at(0, 1) * p_right.at(1, 3)) +
                      (p_left.at(0, 2) * p_right.at(2, 3)) +
                      (p_left.at(0, 3) * p_right.at(3, 3));
  l_return.at(1, 3) = (p_left.at(1, 0) * p_right.at(0, 3)) +
                      (p_left.at(1, 1) * p_right.at(1, 3)) +
                      (p_left.at(1, 2) * p_right.at(2, 3)) +
                      (p_left.at(1, 3) * p_right.at(3, 3));
  l_return.at(2, 3) = (p_left.at(2, 0) * p_right.at(0, 3)) +
                      (p_left.at(2, 1) * p_right.at(1, 3)) +
                      (p_left.at(2, 2) * p_right.at(2, 3)) +
                      (p_left.at(2, 3) * p_right.at(3, 3));
  l_return.at(3, 3) = (p_left.at(3, 0) * p_right.at(0, 3)) +
                      (p_left.at(3, 1) * p_right.at(1, 3)) +
                      (p_left.at(3, 2) * p_right.at(2, 3)) +
                      (p_left.at(3, 3) * p_right.at(3, 3));

  return l_return;
};

template <typename T>
static vec<T, 4> operator*(const mat<T, 4, 4> &p_left,
                           const vec<T, 4> &p_right) {
  vec<T, 4> l_return;
  l_return.at(0) =
      (p_left.at(0, 0) * p_right.at(0)) + (p_left.at(0, 1) * p_right.at(1)) +
      (p_left.at(0, 2) * p_right.at(2)) + (p_left.at(0, 3) * p_right.at(3));
  l_return.at(1) =
      (p_left.at(1, 0) * p_right.at(0)) + (p_left.at(1, 1) * p_right.at(1)) +
      (p_left.at(1, 2) * p_right.at(2)) + (p_left.at(1, 3) * p_right.at(3));
  l_return.at(2) =
      (p_left.at(2, 0) * p_right.at(0)) + (p_left.at(2, 1) * p_right.at(1)) +
      (p_left.at(2, 2) * p_right.at(2)) + (p_left.at(2, 3) * p_right.at(3));
  l_return.at(3) =
      (p_left.at(3, 0) * p_right.at(0)) + (p_left.at(3, 1) * p_right.at(1)) +
      (p_left.at(3, 2) * p_right.at(2)) + (p_left.at(3, 3) * p_right.at(3));

  return l_return;
};

} // namespace m