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

  static mat<T, 4, 4> make_cols(const vec<T, 4> &p_col_0,
                                const vec<T, 4> &p_col_1,
                                const vec<T, 4> &p_col_2,
                                const vec<T, 4> &p_col_3) {
    mat<T, 4, 4> l_return;
    l_return.col0() = p_col_0;
    l_return.col1() = p_col_1;
    l_return.col2() = p_col_2;
    l_return.col3() = p_col_3;
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

template <typename T>
static mat<T, 4, 4> operator*(const mat<T, 4, 4> &p_left, const T &p_right) {
  mat<T, 4, 4> l_return;
  for (auto i = 0; i < 4; ++i) {
    for (auto j = 0; j < 4; ++j) {
      l_return.at(i, j) = p_left.at(i, j) * p_right;
    }
  }

  return l_return;
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

template <typename T> mat<T, 4, 4> inverse(const mat<T, 4, 4> &p_m) {
  T Coef00 = p_m.at(2, 2) * p_m.at(3, 3) - p_m.at(3, 2) * p_m.at(2, 3);
  T Coef02 = p_m.at(1, 2) * p_m.at(3, 3) - p_m.at(3, 2) * p_m.at(1, 3);
  T Coef03 = p_m.at(1, 2) * p_m.at(2, 3) - p_m.at(2, 2) * p_m.at(1, 3);

  T Coef04 = p_m.at(2, 1) * p_m.at(3, 3) - p_m.at(3, 1) * p_m.at(2, 3);
  T Coef06 = p_m.at(1, 1) * p_m.at(3, 3) - p_m.at(3, 1) * p_m.at(1, 3);
  T Coef07 = p_m.at(1, 1) * p_m.at(2, 3) - p_m.at(2, 1) * p_m.at(1, 3);

  T Coef08 = p_m.at(2, 1) * p_m.at(3, 2) - p_m.at(3, 1) * p_m.at(2, 2);
  T Coef10 = p_m.at(1, 1) * p_m.at(3, 2) - p_m.at(3, 1) * p_m.at(1, 2);
  T Coef11 = p_m.at(1, 1) * p_m.at(2, 2) - p_m.at(2, 1) * p_m.at(1, 2);

  T Coef12 = p_m.at(2, 0) * p_m.at(3, 3) - p_m.at(3, 0) * p_m.at(2, 3);
  T Coef14 = p_m.at(1, 0) * p_m.at(3, 3) - p_m.at(3, 0) * p_m.at(1, 3);
  T Coef15 = p_m.at(1, 0) * p_m.at(2, 3) - p_m.at(2, 0) * p_m.at(1, 3);

  T Coef16 = p_m.at(2, 0) * p_m.at(3, 2) - p_m.at(3, 0) * p_m.at(2, 2);
  T Coef18 = p_m.at(1, 0) * p_m.at(3, 2) - p_m.at(3, 0) * p_m.at(1, 2);
  T Coef19 = p_m.at(1, 0) * p_m.at(2, 2) - p_m.at(2, 0) * p_m.at(1, 2);

  T Coef20 = p_m.at(2, 0) * p_m.at(3, 1) - p_m.at(3, 0) * p_m.at(2, 1);
  T Coef22 = p_m.at(1, 0) * p_m.at(3, 1) - p_m.at(3, 0) * p_m.at(1, 1);
  T Coef23 = p_m.at(1, 0) * p_m.at(2, 1) - p_m.at(2, 0) * p_m.at(1, 1);

  vec<T, 4> Fac0 = {Coef00, Coef00, Coef02, Coef03};
  vec<T, 4> Fac1 = {Coef04, Coef04, Coef06, Coef07};
  vec<T, 4> Fac2 = {Coef08, Coef08, Coef10, Coef11};
  vec<T, 4> Fac3 = {Coef12, Coef12, Coef14, Coef15};
  vec<T, 4> Fac4 = {Coef16, Coef16, Coef18, Coef19};
  vec<T, 4> Fac5 = {Coef20, Coef20, Coef22, Coef23};

  vec<T, 4> Vec0 = {p_m.at(1, 0), p_m.at(0, 0), p_m.at(0, 0), p_m.at(0, 0)};
  vec<T, 4> Vec1 = {p_m.at(1, 1), p_m.at(0, 1), p_m.at(0, 1), p_m.at(0, 1)};
  vec<T, 4> Vec2 = {p_m.at(1, 2), p_m.at(0, 2), p_m.at(0, 2), p_m.at(0, 2)};
  vec<T, 4> Vec3 = {p_m.at(1, 3), p_m.at(0, 3), p_m.at(0, 3), p_m.at(0, 3)};

  vec<T, 4> Inv0 = {Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2};
  vec<T, 4> Inv1 = {Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4};
  vec<T, 4> Inv2 = {Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5};
  vec<T, 4> Inv3 = {Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5};

  vec<T, 4> SignA = {+1, -1, +1, -1};
  vec<T, 4> SignB = {-1, +1, -1, +1};
  mat<T, 4, 4> Inverse =
      Inverse.make_cols(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

  vec<T, 4> Row0 = {Inverse.at(0, 0), Inverse.at(1, 0), Inverse.at(2, 0),
                    Inverse.at(3, 0)};

  vec<T, 4> Dot0 = (p_m.col0() * Row0);
  T Dot1 = (Dot0.x() + Dot0.y()) + (Dot0.z() + Dot0.w());

  T OneOverDeterminant = T(1) / Dot1;

  return Inverse * OneOverDeterminant;
};
} // namespace m