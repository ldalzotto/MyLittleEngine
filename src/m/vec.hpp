#pragma once

#include <cmath>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace m {

template <typename T, int N> struct vec {
  T m_data[N];

  T &at(ui8 p_index) { return m_data[p_index]; };

  const T &at(ui8 p_index) const { return m_data[p_index]; };

  vec operator+(const vec<T, N> &p_other) const {
    vec l_return;
    for (auto i = 0; i < N; ++i) {
      l_return.m_data[i] = m_data[i] + p_other.m_data[i];
    }
    return l_return;
  };

  vec &operator+=(const vec<T, N> &p_other) {
    for (auto i = 0; i < N; ++i) {
      m_data[i] = m_data[i] + p_other.m_data[i];
    }
    return *this;
  };

  vec operator-(const vec<T, N> &p_other) const {
    vec l_return;
    for (auto i = 0; i < N; ++i) {
      l_return.m_data[i] = m_data[i] - p_other.m_data[i];
    }
    return l_return;
  };

  vec &operator-=(const vec<T, N> &p_other) {
    for (auto i = 0; i < N; ++i) {
      m_data[i] = m_data[i] - p_other.m_data[i];
    }
    return *this;
  };

  vec operator*(const vec<T, N> &p_other) const {
    vec l_return;
    for (auto i = 0; i < N; ++i) {
      l_return.m_data[i] = m_data[i] * p_other.m_data[i];
    }
    return l_return;
  };

  vec &operator*=(const vec<T, N> &p_other) {
    for (auto i = 0; i < N; ++i) {
      m_data[i] = m_data[i] * p_other.m_data[i];
    }
    return *this;
  };

  vec operator/(const vec<T, N> &p_other) const {
    vec l_return;
    for (auto i = 0; i < N; ++i) {
      l_return.m_data[i] = m_data[i] / p_other.m_data[i];
    }
    return l_return;
  };

  vec operator/(const T &p_other) const {
    vec l_return;
    for (auto i = 0; i < N; ++i) {
      l_return.m_data[i] = m_data[i] / p_other;
    }
    return l_return;
  };

  vec &operator/=(const vec<T, N> &p_other) {
    for (auto i = 0; i < N; ++i) {
      m_data[i] = m_data[i] / p_other.m_data[i];
    }
    return *this;
  };

  static vec getZero() { return {0}; };
};

template <typename T>
vec<T, 3> cross(const vec<T, 3> &p_left, const vec<T, 3> &p_right) {
  return vec<T, 3>{p_left.m_data[1] * p_right.m_data[2] -
                       p_left.m_data[2] * p_right.m_data[1],
                   p_left.m_data[2] * p_right.m_data[0] -
                       p_left.m_data[0] * p_right.m_data[2],
                   p_left.m_data[0] * p_right.m_data[1] -
                       p_left.m_data[1] * p_right.m_data[0]};
};

template <typename T>
f32 dot(const vec<T, 3> &p_left, const vec<T, 3> &p_right) {
  return (p_left.at(0) * p_right.at(0)) + (p_left.at(1) * p_right.at(1)) +
         (p_left.at(2) * p_right.at(2));
};

template <typename T> f32 magnitude(const vec<T, 3> &thiz) {
  return std::sqrt(dot(thiz, thiz));
};

template <typename T> vec<T, 3> normalize(const vec<T, 3> &thiz) {
  return thiz / magnitude(thiz);
};

template <typename T>
T perp_dot(const vec<T, 2> &p_left, const vec<T, 2> &p_right) {
  return (p_left.at(0) * p_right.at(1)) - (p_left.at(1) * p_right.at(0));
};

}; // namespace m