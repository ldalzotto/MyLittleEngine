#pragma once

#include <cor/assertions.hpp>
#include <cor/types.hpp>
#include <m/trig.hpp>
#include <m/vec.hpp>

namespace m {

template <typename T, int R, int C> struct mat;

template <typename T> struct mat<T, 4, 4> {
  vec<T, 4> m_data[4];

  T &at(uimax c, uimax r) { return m_data[c].at(r); }
  const T &at(uimax c, uimax r) const { return m_data[c].at(r); }

  static mat getIdentity() {
    mat<T, 4, 4> l_mat = {0};
    l_mat.at(0, 0) = T(1);
    l_mat.at(1, 1) = T(1);
    l_mat.at(2, 2) = T(1);
    l_mat.at(3, 3) = T(1);
    return l_mat;
  };
  static mat getZero() { return {0}; };

  vec<T, 4> &col0() { return m_data[0]; }
  vec<T, 4> &col1() { return m_data[1]; }
  vec<T, 4> &col2() { return m_data[2]; }
  vec<T, 4> &col3() { return m_data[3]; }
  const vec<T, 4> &col0() const { return m_data[0]; }
  const vec<T, 4> &col1() const { return m_data[1]; }
  const vec<T, 4> &col2() const { return m_data[2]; }
  const vec<T, 4> &col3() const { return m_data[3]; }

  vec<T, 4> &forward() { return col2(); };
  const vec<T, 4> &forward() const { return col2(); };

  static mat<T, 4, 4> make(const mat<T, 3, 3> &p_input) {
    mat<T, 4, 4> l_return;
    l_return.at(0, 0) = p_input.at(0, 0);
    l_return.at(0, 1) = p_input.at(0, 1);
    l_return.at(0, 2) = p_input.at(0, 2);
    l_return.at(0, 3) = 0;

    l_return.at(1, 0) = p_input.at(1, 0);
    l_return.at(1, 1) = p_input.at(1, 1);
    l_return.at(1, 2) = p_input.at(1, 2);
    l_return.at(1, 3) = 0;

    l_return.at(2, 0) = p_input.at(2, 0);
    l_return.at(2, 1) = p_input.at(2, 1);
    l_return.at(2, 2) = p_input.at(2, 2);
    l_return.at(2, 3) = 0;

    l_return.at(3, 0) = 0;
    l_return.at(3, 1) = 0;
    l_return.at(3, 2) = 0;
    l_return.at(3, 3) = 1;

    return l_return;
  };
};

template <typename T> struct mat<T, 3, 3> {
  vec<T, 3> m_data[3];

  T &at(uimax c, uimax r) { return m_data[c].at(r); }
  const T &at(uimax c, uimax r) const { return m_data[c].at(r); }

  static mat getIdentity() {
    mat<T, 3, 3> l_mat = {0};
    l_mat.at(0, 0) = T(1);
    l_mat.at(1, 1) = T(1);
    l_mat.at(2, 2) = T(1);
    return l_mat;
  };
  static mat getZero() { return {0}; };

  vec<T, 3> &col0() { return m_data[0]; }
  vec<T, 3> &col1() { return m_data[1]; }
  vec<T, 3> &col2() { return m_data[2]; }
  const vec<T, 3> &col0() const { return m_data[0]; }
  const vec<T, 3> &col1() const { return m_data[1]; }
  const vec<T, 3> &col2() const { return m_data[2]; }
};

#define col_row(c, r)                                                          \
  (p_left.at(0, c) * p_right.at(r, 0)) +                                       \
      (p_left.at(1, c) * p_right.at(r, 1)) +                                   \
      (p_left.at(2, c) * p_right.at(r, 2)) +                                   \
      (p_left.at(3, c) * p_right.at(r, 3))

template <typename T>
static mat<T, 4, 4> operator*(const mat<T, 4, 4> &p_left,
                              const mat<T, 4, 4> &p_right) {
  mat<T, 4, 4> l_return;

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

#undef col_row

} // namespace m