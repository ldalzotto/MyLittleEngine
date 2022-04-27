#pragma once

#include <cor/types.hpp>

namespace m {

template <typename T, int N> struct vec {
  T m_data[N];

  vec operator+(const vec<T, N> &p_other) {
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

  vec operator-(const vec<T, N> &p_other) {
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

  vec operator*(const vec<T, N> &p_other) {
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

  vec operator/(const vec<T, N> &p_other) {
    vec l_return;
    for (auto i = 0; i < N; ++i) {
      l_return.m_data[i] = m_data[i] / p_other.m_data[i];
    }
    return l_return;
  };

  vec &operator/=(const vec<T, N> &p_other) {
    for (auto i = 0; i < N; ++i) {
      m_data[i] = m_data[i] / p_other.m_data[i];
    }
    return *this;
  };
};

template <typename T>
vec<T, 3> cross(const vec<T, 3> &p_left, const vec<T, 3> &p_right) {
  return vec<T, 3>{p_left.m_data[1] * p_right.m_data[2] -
                       p_left.m_data[2] * p_right.m_data[1],
                   p_left.m_data[0] * p_right.m_data[2] -
                       p_left.m_data[2] * p_right.m_data[0],
                   p_left.m_data[0] * p_right.m_data[1] -
                       p_left.m_data[1] * p_right.m_data[0]};
};

}; // namespace m