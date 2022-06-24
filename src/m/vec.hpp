#pragma once

#include <cmath>
#include <cor/types.hpp>
#include <m/trig.hpp>
#include <sys/sys.hpp>

namespace m {

template <typename T, int N> struct vec;

template <typename T> struct vec<T, 2> {
  T m_data[2];

  FORCE_INLINE T &x() { return m_data[0]; };
  FORCE_INLINE T &y() { return m_data[1]; };
  FORCE_INLINE const T &x() const { return m_data[0]; };
  FORCE_INLINE const T &y() const { return m_data[1]; };

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

  template <typename TT> ui8 operator==(const vec<TT, 2> &p_other) const {
    return x() == p_other.x() && y() == p_other.y();
  };

  static vec getZero() { return {0}; };
};

template <typename T> struct vec<T, 3> {
  T m_data[3];

  inline static const vec left = {1, 0, 0};
  inline static const vec up = {0, 1, 0};
  inline static const vec forward = {0, 0, 1};

  FORCE_INLINE T &x() { return m_data[0]; };
  FORCE_INLINE T &y() { return m_data[1]; };
  FORCE_INLINE T &z() { return m_data[2]; };
  FORCE_INLINE const T &x() const { return m_data[0]; };
  FORCE_INLINE const T &y() const { return m_data[1]; };
  FORCE_INLINE const T &z() const { return m_data[2]; };

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

  template <typename TT> ui8 operator==(const vec<TT, 3> &p_other) const {
    return x() == p_other.x() && y() == p_other.y() && z() == p_other.z();
  };
  template <typename TT> ui8 operator!=(const vec<TT, 3> &p_other) const {
    return !(*this == p_other);
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

  FORCE_INLINE T &x() { return m_data[0]; };
  FORCE_INLINE T &y() { return m_data[1]; };
  FORCE_INLINE T &z() { return m_data[2]; };
  FORCE_INLINE T &w() { return m_data[3]; };
  FORCE_INLINE vec<T, 3> &xyz() { return *(vec<T, 3> *)m_data; };

  FORCE_INLINE const T &x() const { return m_data[0]; };
  FORCE_INLINE const T &y() const { return m_data[1]; };
  FORCE_INLINE const T &z() const { return m_data[2]; };
  FORCE_INLINE const T &w() const { return m_data[3]; };
  FORCE_INLINE const vec<T, 3> &xyz() const {
    return *(const vec<T, 3> *)m_data;
  };

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

}; // namespace m