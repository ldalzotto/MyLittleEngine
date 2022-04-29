#pragma once

#include <cor/assertions.hpp>
#include <cor/types.hpp>

namespace m {

template <typename T, int R, int C> struct mat {
  using value_type = T;
  static constexpr int row_count = R;
  static constexpr int col_count = C;

  T m_data[R * C];

  template <int RR, int CC> T &at() { return m_data[(CC * R) + C]; }
  T &at(uimax r, uimax c) { return m_data[(c * R) + r]; }

  static mat getIdentity();
  static mat getZero() {
    return {0};
  };
};

template <> inline mat<f32, 4, 4> mat<f32, 4, 4>::getIdentity() {
  mat<f32, 4, 4> l_mat = {0};
  l_mat.at(0, 0) = 1;
  l_mat.at(1, 1) = 1;
  l_mat.at(2, 2) = 1;
  l_mat.at(3, 3) = 1;
  return l_mat;
};

} // namespace m