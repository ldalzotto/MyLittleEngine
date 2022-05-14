#pragma once

#include <cmath>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace m {

template <typename T, int N> struct vec;

template <typename T> struct vec<T, 2> {
  T m_data[2];

  T &x() { return m_data[0]; };
  T &y() { return m_data[1]; };
  const T &x() const { return m_data[0]; };
  const T &y() const { return m_data[1]; };

  T &at(ui8 p_index) { return m_data[p_index]; };

  const T &at(ui8 p_index) const { return m_data[p_index]; };

  static vec make(const vec<T, 3> &p_other) {
    vec l_return;
    l_return.m_data[0] = p_other.x();
    l_return.m_data[1] = p_other.y();
    return l_return;
  };

  static vec make(const vec<T, 4> &p_other) {
    vec l_return;
    l_return.m_data[0] = p_other.x();
    l_return.m_data[1] = p_other.y();
    return l_return;
  };

  template <typename TT> vec<TT, 2> cast() {
    return {TT(m_data[0]), TT(m_data[1])};
  };

  vec operator+(const vec<T, 2> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] + p_other.m_data[0];
    l_return.m_data[1] = m_data[1] + p_other.m_data[1];
    return l_return;
  };

  template <typename TT> vec operator+(const TT &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] + p_other;
    l_return.m_data[1] = m_data[1] + p_other;
    return l_return;
  };

  vec &operator+=(const vec<T, 2> &p_other) {
    m_data[0] = m_data[0] + p_other.m_data[0];
    m_data[1] = m_data[1] + p_other.m_data[1];
    return *this;
  };

  vec operator-(const vec<T, 2> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] - p_other.m_data[0];
    l_return.m_data[1] = m_data[1] - p_other.m_data[1];
    return l_return;
  };

  template <typename TT> vec operator-(const TT &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] - p_other;
    l_return.m_data[1] = m_data[1] - p_other;
    return l_return;
  };

  vec &operator-=(const vec<T, 2> &p_other) {
    m_data[0] = m_data[0] - p_other.m_data[0];
    m_data[1] = m_data[1] - p_other.m_data[1];
    return *this;
  };

  template <typename TT> vec operator*(const TT &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] * p_other;
    l_return.m_data[1] = m_data[1] * p_other;
    return l_return;
  };

  vec operator*(const vec<T, 2> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] * p_other.m_data[0];
    l_return.m_data[1] = m_data[1] * p_other.m_data[1];
    return l_return;
  };

  template <typename TT> vec &operator*=(const vec<TT, 2> &p_other) {
    m_data[0] = m_data[0] * p_other.m_data[0];
    m_data[1] = m_data[1] * p_other.m_data[1];
    return *this;
  };

  vec operator/(const vec<T, 2> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] / p_other.m_data[0];
    l_return.m_data[1] = m_data[1] / p_other.m_data[1];
    return l_return;
  };

  vec operator/(const T &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] / p_other;
    l_return.m_data[1] = m_data[1] / p_other;
    return l_return;
  };

  vec &operator/=(const vec<T, 3> &p_other) {
    m_data[0] = m_data[0] / p_other.m_data[0];
    m_data[1] = m_data[1] / p_other.m_data[1];
    return *this;
  };

  static vec getZero() { return {0}; };
};

template <typename T> struct vec<T, 3> {
  T m_data[3];

  T &x() { return m_data[0]; };
  T &y() { return m_data[1]; };
  T &z() { return m_data[2]; };
  const T &x() const { return m_data[0]; };
  const T &y() const { return m_data[1]; };
  const T &z() const { return m_data[2]; };

  static vec make(const vec<T, 4> &p_other) {
    vec l_return;
    l_return.m_data[0] = p_other.x();
    l_return.m_data[1] = p_other.y();
    l_return.m_data[2] = p_other.z();
    return l_return;
  };

  T &at(ui8 p_index) { return m_data[p_index]; };

  const T &at(ui8 p_index) const { return m_data[p_index]; };

  vec operator+(const vec<T, 3> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] + p_other.m_data[0];
    l_return.m_data[1] = m_data[1] + p_other.m_data[1];
    l_return.m_data[2] = m_data[2] + p_other.m_data[2];
    return l_return;
  };

  vec &operator+=(const vec<T, 3> &p_other) {
    m_data[0] = m_data[0] + p_other.m_data[0];
    m_data[1] = m_data[1] + p_other.m_data[1];
    m_data[2] = m_data[2] + p_other.m_data[2];
    return *this;
  };

  vec operator-(const vec<T, 3> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] - p_other.m_data[0];
    l_return.m_data[1] = m_data[1] - p_other.m_data[1];
    l_return.m_data[2] = m_data[2] - p_other.m_data[2];
    return l_return;
  };

  vec operator-(const T &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] - p_other;
    l_return.m_data[1] = m_data[1] - p_other;
    l_return.m_data[2] = m_data[2] - p_other;
    return l_return;
  };

  vec &operator-=(const vec<T, 3> &p_other) {
    m_data[0] = m_data[0] - p_other.m_data[0];
    m_data[1] = m_data[1] - p_other.m_data[1];
    m_data[2] = m_data[2] - p_other.m_data[2];
    return *this;
  };

  vec operator*(const vec<T, 3> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] * p_other.m_data[0];
    l_return.m_data[1] = m_data[1] * p_other.m_data[1];
    l_return.m_data[2] = m_data[2] * p_other.m_data[2];
    return l_return;
  };

  template <typename TT> vec operator*(const TT &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] * p_other;
    l_return.m_data[1] = m_data[1] * p_other;
    l_return.m_data[2] = m_data[2] * p_other;
    return l_return;
  };

  vec &operator*=(const vec<T, 3> &p_other) {
    m_data[0] = m_data[0] * p_other.m_data[0];
    m_data[1] = m_data[1] * p_other.m_data[1];
    m_data[2] = m_data[2] * p_other.m_data[2];
    return *this;
  };

  vec operator/(const vec<T, 3> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] / p_other.m_data[0];
    l_return.m_data[1] = m_data[1] / p_other.m_data[1];
    l_return.m_data[2] = m_data[2] / p_other.m_data[2];
    return l_return;
  };

  vec operator/(const T &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] / p_other;
    l_return.m_data[1] = m_data[1] / p_other;
    l_return.m_data[2] = m_data[2] / p_other;
    return l_return;
  };

  vec &operator/=(const vec<T, 3> &p_other) {
    m_data[0] = m_data[0] / p_other.m_data[0];
    m_data[1] = m_data[1] / p_other.m_data[1];
    m_data[2] = m_data[2] / p_other.m_data[2];
    return *this;
  };

  static vec getZero() { return {0}; };

  template <typename TT> vec<TT, 3> cast() {
    return {TT(m_data[0]), TT(m_data[1]), TT(m_data[2])};
  };
  template <typename TT> const vec<TT, 3> cast() const {
    return {TT(m_data[0]), TT(m_data[1]), TT(m_data[2])};
  };
};

template <typename T> struct vec<T, 4> {
  T m_data[4];

  T &x() { return m_data[0]; };
  T &y() { return m_data[1]; };
  T &z() { return m_data[2]; };
  T &w() { return m_data[3]; };

  const T &x() const { return m_data[0]; };
  const T &y() const { return m_data[1]; };
  const T &z() const { return m_data[2]; };
  const T &w() const { return m_data[3]; };

  T &at(ui8 p_index) { return m_data[p_index]; };

  const T &at(ui8 p_index) const { return m_data[p_index]; };

  vec operator+(const vec<T, 4> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] + p_other.m_data[0];
    l_return.m_data[1] = m_data[1] + p_other.m_data[1];
    l_return.m_data[2] = m_data[2] + p_other.m_data[2];
    l_return.m_data[3] = m_data[3] + p_other.m_data[3];
    return l_return;
  };

  vec &operator+=(const vec<T, 4> &p_other) {
    m_data[0] = m_data[0] + p_other.m_data[0];
    m_data[1] = m_data[1] + p_other.m_data[1];
    m_data[2] = m_data[2] + p_other.m_data[2];
    m_data[3] = m_data[3] + p_other.m_data[3];
    return *this;
  };

  vec operator-(const vec<T, 4> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] - p_other.m_data[0];
    l_return.m_data[1] = m_data[1] - p_other.m_data[1];
    l_return.m_data[2] = m_data[2] - p_other.m_data[2];
    l_return.m_data[3] = m_data[3] - p_other.m_data[3];
    return l_return;
  };

  vec &operator-=(const vec<T, 4> &p_other) {
    m_data[0] = m_data[0] - p_other.m_data[0];
    m_data[1] = m_data[1] - p_other.m_data[1];
    m_data[2] = m_data[2] - p_other.m_data[2];
    m_data[3] = m_data[3] - p_other.m_data[3];
    return *this;
  };

  vec operator*(const vec<T, 4> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] * p_other.m_data[0];
    l_return.m_data[1] = m_data[1] * p_other.m_data[1];
    l_return.m_data[2] = m_data[2] * p_other.m_data[2];
    l_return.m_data[3] = m_data[3] * p_other.m_data[3];
    return l_return;
  };

  vec operator*(const T &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] * p_other;
    l_return.m_data[1] = m_data[1] * p_other;
    l_return.m_data[2] = m_data[2] * p_other;
    l_return.m_data[3] = m_data[3] * p_other;
    return l_return;
  };

  vec &operator*=(const vec<T, 4> &p_other) {
    m_data[0] = m_data[0] * p_other.m_data[0];
    m_data[1] = m_data[1] * p_other.m_data[1];
    m_data[2] = m_data[2] * p_other.m_data[2];
    m_data[3] = m_data[3] * p_other.m_data[3];
    return *this;
  };

  vec operator/(const vec<T, 4> &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] / p_other.m_data[0];
    l_return.m_data[1] = m_data[1] / p_other.m_data[1];
    l_return.m_data[2] = m_data[2] / p_other.m_data[2];
    l_return.m_data[3] = m_data[3] / p_other.m_data[3];
    return l_return;
  };

  vec operator/(const T &p_other) const {
    vec l_return;
    l_return.m_data[0] = m_data[0] / p_other;
    l_return.m_data[1] = m_data[1] / p_other;
    l_return.m_data[2] = m_data[2] / p_other;
    l_return.m_data[3] = m_data[3] / p_other;
    return l_return;
  };

  vec &operator/=(const vec<T, 4> &p_other) {
    m_data[0] = m_data[0] / p_other.m_data[0];
    m_data[1] = m_data[1] / p_other.m_data[1];
    m_data[2] = m_data[2] / p_other.m_data[2];
    m_data[3] = m_data[3] / p_other.m_data[3];
    return *this;
  };

  static vec getZero() { return {0}; };

  static vec make(const vec<T, 3> &p_xyz, const T &p_w) {
    return {p_xyz.x(), p_xyz.y(), p_xyz.z(), p_w};
  };
};

template <typename T>
T cross(const vec<T, 2> &p_left, const vec<T, 2> &p_right) {
  return p_left.x() * p_right.y() - p_left.y() * p_right.x();
};

template <typename T>
vec<T, 3> cross(const vec<T, 3> &p_left, const vec<T, 3> &p_right) {
  return vec<T, 3>{p_left.y() * p_right.z() - p_left.z() * p_right.y(),
                   p_left.z() * p_right.x() - p_left.x() * p_right.z(),
                   p_left.x() * p_right.y() - p_left.y() * p_right.x()};
};

template <typename T> T dot(const vec<T, 2> &p_left, const vec<T, 2> &p_right) {
  return (p_left.x() * p_right.x()) + (p_left.y() * p_right.y());
};

template <typename T> T dot(const vec<T, 3> &p_left, const vec<T, 3> &p_right) {
  return (p_left.x() * p_right.x()) + (p_left.y() * p_right.y()) +
         (p_left.z() * p_right.z());
};

template <typename T> f32 magnitude(const vec<T, 3> &thiz) {
  return std::sqrt(dot(thiz, thiz));
};

template <typename T> vec<T, 3> normalize(const vec<T, 3> &thiz) {
  return thiz / magnitude(thiz);
};

template <typename T> ui8 is_normalized(const vec<T, 3> &thiz) {
  f32 l_magnitude = magnitude(thiz);
  return l_magnitude >= 0.99 && l_magnitude <= 1.01;
};

template <typename T>
T perp_dot(const vec<T, 2> &p_left, const vec<T, 2> &p_right) {
  return (p_left.x() * p_right.y()) - (p_left.y() * p_right.x());
};

}; // namespace m