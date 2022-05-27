#pragma once

#include <cmath>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace m {

template <ui8 ScaleFactor> struct fixed {

  static constexpr ui8 scale_factor = ScaleFactor;
  static constexpr ui32 scale = 1 << scale_factor;

  // static constexpr fixed pi_8();
  static constexpr fixed pi_8();
  static const fixed pi_4;
  static const fixed pi_2;
  static const fixed pi;
  static const fixed e;

  static constexpr i32 whole_mask = 0xFFFFFFFF << scale_factor;

  i32 m_value;

  fixed() = default;
  constexpr fixed(f32 p_value);
  constexpr fixed(f64 p_value);
  constexpr fixed(const fixed &p_value);

  constexpr fixed(ui8 p_value);
  constexpr fixed(ui16 p_value);
  constexpr fixed(ui32 p_value);
  constexpr fixed(ui64 p_value);
  constexpr fixed(i8 p_value);
  constexpr fixed(i16 p_value);
  constexpr fixed(i32 p_value);
  constexpr fixed(i64 p_value);
  constexpr f32 to_f32() { return (f32)m_value / scale; };

  constexpr fixed operator+(fixed p_other) const;
  constexpr fixed &operator+=(fixed p_other);
  constexpr fixed operator-(fixed p_other) const;
  constexpr fixed &operator-=(fixed p_other);
  constexpr fixed operator*(fixed p_other) const;
  constexpr fixed &operator*=(fixed p_other);
  constexpr fixed operator/(fixed p_other) const;
  constexpr fixed operator%(fixed p_other) const;
  constexpr ui8 operator>=(fixed p_other) const;
  constexpr ui8 operator>(fixed p_other) const;
  constexpr ui8 operator<=(fixed p_other) const;
  constexpr ui8 operator<(fixed p_other) const;
  constexpr ui8 operator==(fixed p_other) const;

  constexpr fixed abs() const;
};

template <ui8 ScaleFactor>
inline constexpr fixed<ScaleFactor> fixed<ScaleFactor>::pi_8() {
  return fixed<ScaleFactor>(3.14159265358979323846 / 8);
};

template <ui8 ScaleFactor>
inline constexpr fixed<ScaleFactor> fixed<ScaleFactor>::pi_4 =
    fixed<ScaleFactor>::pi_8() * fixed<ScaleFactor>(2);

template <ui8 ScaleFactor>
inline constexpr fixed<ScaleFactor> fixed<ScaleFactor>::pi_2 =
    fixed<ScaleFactor>::pi_8() * fixed<ScaleFactor>(4);

template <ui8 ScaleFactor>
inline constexpr fixed<ScaleFactor>
    fixed<ScaleFactor>::pi = fixed<ScaleFactor>::pi_8() * fixed<ScaleFactor>(8);

template <ui8 ScaleFactor>
inline constexpr fixed<ScaleFactor>
    fixed<ScaleFactor>::e = fixed<ScaleFactor>(2.7182818284590);

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>::fixed(const fixed &p_value) {
  m_value = p_value.m_value;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>::fixed(f32 p_value) {
  m_value = sys::nearest(p_value * fixed<ScaleFactor>::scale);
};
template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>::fixed(f64 p_value) {
  m_value = sys::nearest(p_value * fixed<ScaleFactor>::scale);
};
template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>::fixed(ui8 p_value) {
  m_value = fixed<ScaleFactor>::scale * p_value;
};
template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>::fixed(ui16 p_value) {
  m_value = fixed<ScaleFactor>::scale * p_value;
};
template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>::fixed(ui32 p_value) {
  m_value = fixed<ScaleFactor>::scale * p_value;
};
template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>::fixed(i8 p_value) {
  m_value = fixed<ScaleFactor>::scale * p_value;
};
template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>::fixed(i16 p_value) {
  m_value = fixed<ScaleFactor>::scale * p_value;
};
template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>::fixed(i32 p_value) {
  m_value = fixed<ScaleFactor>::scale * p_value;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>
fixed<ScaleFactor>::operator+(fixed p_other) const {
  fixed l_value_fixed;
  l_value_fixed.m_value = m_value + p_other.m_value;
  return l_value_fixed;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor> &
fixed<ScaleFactor>::operator+=(fixed p_other) {
  m_value += p_other.m_value;
  return *this;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>
fixed<ScaleFactor>::operator-(fixed p_other) const {
  fixed l_value_fixed;
  l_value_fixed.m_value = m_value - p_other.m_value;
  return l_value_fixed;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor> &
fixed<ScaleFactor>::operator-=(fixed p_other) {
  m_value -= p_other.m_value;
  return *this;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>
fixed<ScaleFactor>::operator*(fixed p_other) const {
  fixed<ScaleFactor> l_value = fixed<ScaleFactor>(0);
  l_value.m_value =(((long long)m_value * (long long)p_other.m_value) + (scale >> 1)) >>
      scale_factor;  
  return l_value;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor> &
fixed<ScaleFactor>::operator*=(fixed p_other) {
  *this = *this * p_other;
  return *this;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>
fixed<ScaleFactor>::operator/(fixed p_other) const {
  fixed l_value_fixed;
  l_value_fixed.m_value =
      (((((long long)m_value) * scale) + (scale >> 1))) / p_other.m_value;
  return l_value_fixed;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor>
fixed<ScaleFactor>::operator%(fixed p_other) const {
  fixed l_value_fixed;
  l_value_fixed.m_value = m_value % p_other.m_value;
  return l_value_fixed;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr ui8 fixed<ScaleFactor>::operator>=(fixed p_other) const {
  return m_value >= p_other.m_value;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr ui8 fixed<ScaleFactor>::operator>(fixed p_other) const {
  return m_value > p_other.m_value;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr ui8 fixed<ScaleFactor>::operator<=(fixed p_other) const {
  return m_value <= p_other.m_value;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr ui8 fixed<ScaleFactor>::operator<(fixed p_other) const {
  return m_value < p_other.m_value;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr ui8 fixed<ScaleFactor>::operator==(fixed p_other) const {
  return m_value == p_other.m_value;
};

template <ui8 ScaleFactor>
FORCE_INLINE constexpr fixed<ScaleFactor> fixed<ScaleFactor>::abs() const {
  fixed<ScaleFactor> l_fixed;
  l_fixed.m_value = std::abs(m_value);
  return l_fixed;
};

} // namespace m

using ff32 = m::fixed<10>;
