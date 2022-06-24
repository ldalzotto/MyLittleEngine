#pragma once

#include <cor/types.hpp>
#include <m/vec.hpp>

namespace m {

template <typename T> struct quat {
  T m_data[4];

  T &x() { return m_data[0]; };
  T &y() { return m_data[1]; };
  T &z() { return m_data[2]; };
  T &w() { return m_data[3]; };
  const T &x() const { return m_data[0]; };
  const T &y() const { return m_data[1]; };
  const T &z() const { return m_data[2]; };
  const T &w() const { return m_data[3]; };

  static quat getIdentity() {
    quat l_quat;
    l_quat.x() = 0;
    l_quat.y() = 0;
    l_quat.z() = 0;
    l_quat.w() = 1;
    return l_quat;
  };

  static quat make_wxyz(T w, T x, T y, T z) {
    quat l_quat;
    l_quat.w() = w;
    l_quat.x() = x;
    l_quat.y() = y;
    l_quat.z() = z;
    return l_quat;
  };

  static quat make_w_xyz(T p_w, const vec<T, 3> &p_xyz) {
    quat l_quat;
    l_quat.w() = p_w;
    l_quat.x() = p_xyz.x();
    l_quat.y() = p_xyz.y();
    l_quat.z() = p_xyz.z();
    return l_quat;
  };
};

template <typename T>
ui8 operator==(const quat<T> &p_left, const quat<T> &p_right) {
  return (p_left.x() == p_right.x()) && (p_left.y() == p_right.y()) &&
         (p_left.z() == p_right.z()) && (p_left.w() == p_right.w());
};

template <typename T>
ui8 operator!=(const quat<T> &p_left, const quat<T> &p_right) {
  return !(p_left == p_right);
};

template <typename T>
quat<T> operator*(const quat<T> &p_left, const quat<T> &p_right) {
  quat<T> l_return;

  l_return.w() = p_left.w() * p_right.w() - p_left.x() * p_right.x() -
                 p_left.y() * p_right.y() - p_left.z() * p_right.z();
  l_return.x() = p_left.w() * p_right.x() + p_left.x() * p_right.w() +
                 p_left.y() * p_right.z() - p_left.z() * p_right.y();
  l_return.y() = p_left.w() * p_right.y() + p_left.y() * p_right.w() +
                 p_left.z() * p_right.x() - p_left.x() * p_right.z();
  l_return.z() = p_left.w() * p_right.z() + p_left.z() * p_right.w() +
                 p_left.x() * p_right.y() - p_left.y() * p_right.x();
  return l_return;
};

template <typename T>
quat<T> &operator*=(quat<T> &p_left, const quat<T> &p_right) {
  p_left = p_left * p_right;
  return p_left;
};

} // namespace m