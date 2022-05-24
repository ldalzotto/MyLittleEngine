#pragma once

#include <cmath>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace m {

template <ui8 ScaleFactor> struct fixed {

  template <typename Int> FORCE_INLINE static fixed make_no_scale(Int p_value) {
    fixed l_fixed;
    l_fixed.m_value = p_value;
    return l_fixed;
  };

  static constexpr ui8 scale_factor = ScaleFactor;
  static constexpr ui32 scale = 1 << scale_factor;

  inline static constexpr fixed one() {
    fixed l_fixed;
    l_fixed.m_value = scale;
    return l_fixed;
  };

  // inline static const fixed one = make_no_scale(scale);
  inline static const fixed pi_8 = fixed(3.14159265358979323846 / 8) * one();
  inline static const fixed pi_4 = pi_8 * 2;
  inline static const fixed pi_2 = pi_8 * 4;
  inline static const fixed pi = pi_8 * 8;
  inline static const fixed e = 2.7182818284590;

  static const i32 whole_mask = 0xFFFFFFFF << scale_factor;

  i32 m_value;

  fixed() = default;
  FORCE_INLINE fixed(f32 p_value) {
    m_value = sys::nearest(p_value * one().m_value);
  };
  FORCE_INLINE fixed(f64 p_value) {
    m_value = sys::nearest(p_value * one().m_value);
  };
  FORCE_INLINE fixed(const fixed &p_value) { *this = p_value; };

  FORCE_INLINE fixed(ui8 p_value) { m_value = one().m_value * p_value; };
  FORCE_INLINE fixed(ui16 p_value) { m_value = one().m_value * p_value; };
  FORCE_INLINE fixed(ui32 p_value) { m_value = one().m_value * p_value; };
  FORCE_INLINE fixed(ui64 p_value) { m_value = one().m_value * p_value; };
  FORCE_INLINE fixed(i8 p_value) { m_value = one().m_value * p_value; };
  FORCE_INLINE fixed(i16 p_value) { m_value = one().m_value * p_value; };
  FORCE_INLINE fixed(i32 p_value) { m_value = one().m_value * p_value; };
  FORCE_INLINE fixed(i64 p_value) { m_value = one().m_value * p_value; };
  f32 to_f32() { return (f32)m_value / scale; };

  FORCE_INLINE
  fixed operator+(fixed p_other) const {
    fixed l_value_fixed;
    l_value_fixed.m_value = m_value + p_other.m_value;
    return l_value_fixed;
  };
  FORCE_INLINE
  fixed &operator+=(fixed p_other) {
    m_value += p_other.m_value;
    return *this;
  };
  FORCE_INLINE
  fixed operator-(fixed p_other) const {
    fixed l_value_fixed;
    l_value_fixed.m_value = m_value - p_other.m_value;
    return l_value_fixed;
  };
  FORCE_INLINE
  fixed &operator-=(fixed p_other) {
    m_value -= p_other.m_value;
    return *this;
  };
  FORCE_INLINE
  fixed operator*(fixed p_other) const {
    fixed l_value_fixed;
    l_value_fixed.m_value =
        (((long long)m_value * (long long)p_other.m_value) + (scale >> 1)) >>
        scale_factor;
    return l_value_fixed;
  };
  FORCE_INLINE
  fixed &operator*=(fixed p_other) {
    *this = *this * p_other;
    return *this;
  };
  FORCE_INLINE
  fixed operator/(fixed p_other) const {
    fixed l_value_fixed;
    l_value_fixed.m_value =
        (((((long long)m_value) * scale) + (scale >> 1))) / p_other.m_value;
    return l_value_fixed;
  };
  FORCE_INLINE
  fixed operator%(fixed p_other) const {
    fixed l_value_fixed;
    l_value_fixed.m_value = m_value % p_other.m_value;
    return l_value_fixed;
  };
  FORCE_INLINE
  ui8 operator>=(fixed p_other) const { return m_value >= p_other.m_value; };
  FORCE_INLINE
  ui8 operator>(fixed p_other) const { return m_value > p_other.m_value; };
  FORCE_INLINE
  ui8 operator<=(fixed p_other) const { return m_value <= p_other.m_value; };
  FORCE_INLINE
  ui8 operator<(fixed p_other) const { return m_value < p_other.m_value; };
  FORCE_INLINE
  ui8 operator==(fixed p_other) const { return m_value == p_other.m_value; };
};

template <ui8 ScaleFactor> fixed<ScaleFactor> abs(fixed<ScaleFactor> p_value) {
  fixed<ScaleFactor> l_fixed;
  l_fixed.m_value = std::abs(p_value.m_value);
  return l_fixed;
};

} // namespace m

using ff32 = m::fixed<10>;
