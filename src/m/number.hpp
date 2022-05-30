#pragma once
#include <cor/traits.hpp>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace m {

enum class NumberType { Integer = 0, Floating = 1, Fixed = 2, Undefined = 3 };

template <NumberType Type> struct number_type_dispatcher { using type = ui8; };

template <typename T> struct is_builtin { static constexpr ui8 value = 0; };
template <> struct is_builtin<ui8> { static constexpr ui8 value = 1; };
template <> struct is_builtin<i8> { static constexpr ui8 value = 1; };
template <> struct is_builtin<ui16> { static constexpr ui8 value = 1; };
template <> struct is_builtin<i16> { static constexpr ui8 value = 1; };
template <> struct is_builtin<ui32> { static constexpr ui8 value = 1; };
template <> struct is_builtin<i32> { static constexpr ui8 value = 1; };
template <> struct is_builtin<f32> { static constexpr ui8 value = 1; };
template <> struct is_builtin<f64> { static constexpr ui8 value = 1; };

template <ui8 Value> struct is_builtin_dispatcher {};

template <typename T> struct is_builtin_floating {
  static constexpr ui8 value = 0;
};
template <> struct is_builtin_floating<f32> { static constexpr ui8 value = 1; };
template <> struct is_builtin_floating<f64> { static constexpr ui8 value = 1; };

template <typename T> struct is_fixed { static constexpr ui8 value = 0; };

template <typename T>
constexpr NumberType get_number_type(
    enable_if_t<is_builtin<T>::value && is_builtin_floating<T>::value, void *> =
        0) {
  return NumberType::Floating;
};

template <typename T>
constexpr NumberType get_number_type(
    enable_if_t<is_builtin<T>::value && !is_builtin_floating<T>::value,
                void *> = 0) {
  return NumberType::Integer;
};

template <typename T>
constexpr NumberType
get_number_type(enable_if_t<is_fixed<T>::value, void *> = 0) {
  return NumberType::Fixed;
};

}; // namespace m

namespace m {

template <typename T, ui8 ScaleFactor> struct fixed {

  static constexpr ui8 scale_factor = ScaleFactor;
  static constexpr ui32 scale = 1 << scale_factor;

  T m_value;

  FORCE_INLINE fixed() = default;

  FORCE_INLINE constexpr f32 to_f32() { return (f32)m_value / scale; };

  template <typename TT> FORCE_INLINE constexpr fixed(TT p_other) {
    __make(p_other);
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed operator+(TT p_other) const {
    return __add(p_other);
  };

  template <typename TT> FORCE_INLINE constexpr fixed &operator+=(TT p_other) {
    *this = *this + p_other;
    return *this;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed operator-(TT p_other) const {
    return __sub(p_other);
  };

  template <typename TT> FORCE_INLINE constexpr fixed &operator-=(TT p_other) {
    *this = *this - p_other;
    return *this;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed operator*(TT p_other) const {
    return __mul(p_other);
  };

  template <typename TT> FORCE_INLINE constexpr fixed &operator*=(TT p_other) {
    *this = *this * p_other;
    return *this;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed operator/(TT p_other) const {
    return __div(p_other);
  };

  template <typename TT> FORCE_INLINE constexpr fixed &operator/=(TT p_other) {
    *this = *this / p_other;
    return *this;
  };

  template <typename TT>
  FORCE_INLINE constexpr ui8 operator>=(TT p_other) const {
    return m_value >= fixed(p_other).m_value;
  };
  template <typename TT>
  FORCE_INLINE constexpr ui8 operator>(TT p_other) const {
    return m_value > fixed(p_other).m_value;
  };
  template <typename TT>
  FORCE_INLINE constexpr ui8 operator<=(TT p_other) const {
    return m_value <= fixed(p_other).m_value;
  };
  template <typename TT>
  FORCE_INLINE constexpr ui8 operator<(TT p_other) const {
    return m_value < fixed(p_other).m_value;
  };

  template <typename TT>
  FORCE_INLINE constexpr ui8 operator==(TT p_other) const {
    return m_value == fixed(p_other).m_value;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed operator%(TT p_other) const {
    fixed l_value = 0;
    l_value.m_value = m_value % fixed(p_other).m_value;
    return l_value;
  };

private:
  template <typename TT>
  FORCE_INLINE constexpr void __make(
      TT p_value,
      enable_if_t<get_number_type<TT>() == NumberType::Floating, void *> = 0) {
    m_value = sys::nearest(p_value * scale);
  };

  template <typename TT>
  FORCE_INLINE constexpr void __make(
      TT p_value,
      enable_if_t<get_number_type<TT>() == NumberType::Integer, void *> = 0) {
    m_value = i32(p_value) << ScaleFactor;
  };

  template <typename TT>
  FORCE_INLINE constexpr void
  __make(TT p_value,
         enable_if_t<get_number_type<TT>() == NumberType::Fixed, void *> = 0) {
    *this = p_value;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __add(TT p_value,
        enable_if_t<get_number_type<TT>() == NumberType::Fixed, void *> =
            0) const {
    fixed l_value = 0;
    l_value.m_value = m_value + p_value.m_value;
    return l_value;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __add(TT p_value,
        enable_if_t<get_number_type<TT>() != NumberType::Fixed, void *> =
            0) const {
    return __add(fixed(p_value));
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __sub(TT p_value,
        enable_if_t<get_number_type<TT>() == NumberType::Fixed, void *> =
            0) const {
    fixed l_value = 0;
    l_value.m_value = m_value - p_value.m_value;
    return l_value;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __sub(TT p_value,
        enable_if_t<get_number_type<TT>() != NumberType::Fixed, void *> =
            0) const {
    return __sub(fixed(p_value));
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __mul(TT p_value,
        enable_if_t<get_number_type<TT>() == NumberType::Fixed, void *> =
            0) const {
    fixed l_fixed = 0;
    l_fixed.m_value =
        (((long long)m_value * p_value.m_value) + (scale >> 1)) >> ScaleFactor;
    return l_fixed;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __mul(TT p_value,
        enable_if_t<get_number_type<TT>() != NumberType::Fixed, void *> =
            0) const {
    return __mul(fixed(p_value));
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __div(TT p_value,
        enable_if_t<get_number_type<TT>() == NumberType::Fixed, void *> =
            0) const {
    fixed l_fixed = 0;
    l_fixed.m_value =
        (((((long long)m_value) * scale) + (scale >> 1))) / p_value.m_value;
    return l_fixed;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __div(TT p_value,
        enable_if_t<get_number_type<TT>() != NumberType::Fixed, void *> =
            0) const {
    return __div(fixed(p_value));
  };
};

template <typename T, ui8 ScaleFactor> struct is_fixed<fixed<T, ScaleFactor>> {
  static constexpr ui8 value = 1;
};

}; // namespace m

using fix32 = m::fixed<i32, 10>;