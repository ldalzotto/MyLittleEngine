#pragma once

#include <cor/traits.hpp>
#include <float.h>
#include <m/number.hpp>
#include <sys/sys.hpp>

namespace m {

template <typename T, NumberType Type = get_number_type<T>()> struct epsilon {};

template <typename T> struct epsilon<T, NumberType::Fixed> {
  static FORCE_INLINE constexpr T value() {
    T l_value = 0;
    l_value.m_value = 1;
    return l_value;
  };
};

template <> struct epsilon<f32> {
  static FORCE_INLINE constexpr f32 value() { return f32(FLT_EPSILON); };
};

template <typename T>
static FORCE_INLINE constexpr T
abs(T v,
    traits::enable_if_t<m::get_number_type<T>() != NumberType::Fixed, void *> =
        0) {
  return v >= 0 ? v : (v * -1);
};

template <typename T>
static FORCE_INLINE constexpr T
abs(T v,
    traits::enable_if_t<m::get_number_type<T>() == NumberType::Fixed, void *> =
        0) {
  T l_value;
  l_value.m_value = m::abs(v.m_value);
  return l_value;
};

template <typename T> FORCE_INLINE static constexpr T pi_8() {
  return 3.14159265358979323846 / 8;
};

template <typename T> FORCE_INLINE static constexpr T pi_4() {
  return pi_8<T>() * 2;
};
template <typename T> FORCE_INLINE static constexpr T pi_2() {
  return pi_8<T>() * 4;
};
template <typename T> FORCE_INLINE static constexpr T pi() {
  return pi_8<T>() * 8;
};
template <typename T> FORCE_INLINE static constexpr T e() {
  return 2.7182818284590;
};

template <typename T> static FORCE_INLINE constexpr i32 nearest(T v) {
  i32 lx = i32(v);
  i32 lxr = i32(v + (T(0.5f) * (v > 0)));
  if (lxr == lx) {
    return lx;
  }
  return lxr;
};

} // namespace m
