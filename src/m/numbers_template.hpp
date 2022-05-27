#pragma once
#include <cor/types.hpp>
#include <m/fixed_utils.hpp>
#include <sys/sys.hpp>

namespace m {

enum class NumberType { Integer = 0, Floating = 1, Fixed = 2 };

template <typename T> constexpr NumberType get_number_type();

#if 1
template <NumberType Type> struct number_type_dispatcher {};
#endif

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

#if 0
template <typename T, NumberType _Type> struct number_v3 {

  static constexpr NumberType Type = _Type;

  T m_value;
  FORCE_INLINE constexpr T &value() { return m_value; };
  FORCE_INLINE constexpr const T &value() const { return m_value; };

  template <typename TT> FORCE_INLINE constexpr number_v3(TT p_value) {
    __number_v3_constructor(p_value, number_type_dispatcher<Type>(),
                            is_builtin_dispatcher<is_builtin<TT>::value>());
  };

  template <typename TT>
  FORCE_INLINE constexpr number_v3 operator+(TT p_value) const {
    return __add(p_value, number_type_dispatcher<Type>(),
                 is_builtin_dispatcher<is_builtin<TT>::value>());
  };

private:
  template <typename TT, typename NumberTypeDispatcher>
  FORCE_INLINE constexpr void
  __number_v3_constructor(TT p_value, NumberTypeDispatcher,
                          is_builtin_dispatcher<1>) {
    m_value = p_value;
  };

  template <typename TT, typename NumberTypeDispatcher>
  FORCE_INLINE constexpr void
  __number_v3_constructor(TT p_value, NumberTypeDispatcher,
                          is_builtin_dispatcher<0>) {
    m_value = p_value.value();
  };

  template <typename TT, typename NumberTypeDispatcher>
  FORCE_INLINE constexpr number_v3 __add(TT p_value, NumberTypeDispatcher,
                                         is_builtin_dispatcher<1>) const {
    return m_value + p_value;
  };

  template <typename TT, typename NumberTypeDispatcher>
  FORCE_INLINE constexpr number_v3 __add(TT p_value, NumberTypeDispatcher,
                                         is_builtin_dispatcher<0>) const {
    return m_value + p_value.value();
  };
};

#endif
}; // namespace m

namespace m {

template <typename T, ui8 ScaleFactor> struct fixed {

  static constexpr ui8 scale_factor = ScaleFactor;
  static constexpr ui32 scale = 1 << scale_factor;

  T m_value;

  FORCE_INLINE static constexpr fixed pi_8() { return 3.14159265358979323846 / 8; };
  FORCE_INLINE static constexpr fixed pi_4() { return pi_8() * 2; };
  FORCE_INLINE static constexpr fixed pi_2() { return pi_8() * 4; };
  FORCE_INLINE static constexpr fixed pi() { return pi_8() * 8; };
  FORCE_INLINE static constexpr fixed e() { return 2.7182818284590; };

  FORCE_INLINE fixed() = default;
  FORCE_INLINE constexpr const T &value() const { return m_value; };

  FORCE_INLINE constexpr f32 to_f32() { return (f32)m_value / scale; };

  template <typename TT> FORCE_INLINE constexpr fixed(TT p_other) {
    __make(p_other, is_builtin_dispatcher<is_builtin<TT>::value>(),
           number_type_dispatcher<get_number_type<TT>()>());
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed operator+(TT p_other) const {
    fixed l_value = 0;
    l_value.m_value = m_value + fixed(p_other).value();
    return l_value;
  };

  template <typename TT> FORCE_INLINE constexpr fixed &operator+=(TT p_other) {
    *this = *this + p_other;
    return *this;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed operator-(TT p_other) const {
    fixed l_value = 0;
    l_value.m_value = m_value - fixed(p_other).value();
    return l_value;
  };

  template <typename TT> FORCE_INLINE constexpr fixed &operator-=(TT p_other) {
    *this = *this - p_other;
    return *this;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed operator*(TT p_other) const {
    return __mul(p_other, is_builtin_dispatcher<is_builtin<TT>::value>(),
                 number_type_dispatcher<get_number_type<TT>()>());
  };

  template <typename TT> FORCE_INLINE constexpr fixed &operator*=(TT p_other) {
    *this = *this * p_other;
    return *this;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed operator/(TT p_other) const {
    return __div(p_other, is_builtin_dispatcher<is_builtin<TT>::value>(),
                 number_type_dispatcher<get_number_type<TT>()>());
  };

  template <typename TT> FORCE_INLINE constexpr fixed &operator/=(TT p_other) {
    *this = *this / p_other;
    return *this;
  };

  template <typename TT>
  FORCE_INLINE constexpr ui8 operator>=(TT p_other) const {
    return m_value >= fixed(p_other).value();
  };
  template <typename TT>
  FORCE_INLINE constexpr ui8 operator>(TT p_other) const {
    return m_value > fixed(p_other).value();
  };
  template <typename TT>
  FORCE_INLINE constexpr ui8 operator<=(TT p_other) const {
    return m_value <= fixed(p_other).value();
  };
  template <typename TT>
  FORCE_INLINE constexpr ui8 operator<(TT p_other) const {
      return m_value < fixed(p_other).value();
  };

  template <typename TT>
  FORCE_INLINE constexpr ui8 operator==(TT p_other) const {
      return m_value == fixed(p_other).value();
  };


  template <typename TT>
  FORCE_INLINE constexpr fixed operator%(TT p_other) const {
    fixed l_value = 0;
    l_value.m_value = m_value % fixed(p_other).value();
    return l_value;
  };

  FORCE_INLINE constexpr fixed abs() const {
    fixed l_value = 0;
    l_value.m_value = sys::abs(m_value);
    return l_value;
  };

private:
  template <typename TT>
  FORCE_INLINE constexpr void
  __make(TT p_value, is_builtin_dispatcher<1>,
         number_type_dispatcher<NumberType::Floating>) {
    fixed_utils::make_floating_builtin(*this, p_value);
  };

  template <typename TT>
  FORCE_INLINE constexpr void
  __make(TT p_value, is_builtin_dispatcher<1>,
         number_type_dispatcher<NumberType::Integer>) {
    fixed_utils::make_integer_builtin(*this, p_value);
  };

  template <typename TT>
  FORCE_INLINE constexpr void
  __make(TT p_value, is_builtin_dispatcher<0>,
         number_type_dispatcher<NumberType::Fixed>) {
    *this = p_value;
  };

  template <typename TT, typename BuiltinDispatcher,
            typename NumberTypeDispatcher>
  FORCE_INLINE constexpr fixed __mul(TT p_value, BuiltinDispatcher,
                                     NumberTypeDispatcher) const {
    fixed l_fixed = 0;
    l_fixed.m_value = fixed_utils::mult_number(*this, p_value);
    return l_fixed;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __mul(TT p_value, is_builtin_dispatcher<0>,
        number_type_dispatcher<NumberType::Fixed>) const {
    fixed l_fixed = 0;
    l_fixed.m_value = fixed_utils::mult_fixed(*this, p_value);
    return l_fixed;
  };

  template <typename TT, typename BuiltinDispatcher,
            typename NumberTypeDispatcher>
  FORCE_INLINE constexpr fixed __div(TT p_value, BuiltinDispatcher,
                                     NumberTypeDispatcher) const {
    fixed l_fixed = 0;
    l_fixed.m_value = fixed_utils::div_number(*this, p_value);
    return l_fixed;
  };

  template <typename TT>
  FORCE_INLINE constexpr fixed
  __div(TT p_value, is_builtin_dispatcher<0>,
        number_type_dispatcher<NumberType::Fixed>) const {
    fixed l_fixed = 0;
    l_fixed.m_value = fixed_utils::div_fixed(*this, p_value);
    return l_fixed;
  };


#if 0

  static constexpr number_fixed32 pi_8();
  static constexpr number_fixed32 pi_4();
  static constexpr number_fixed32 pi_2();
  static constexpr number_fixed32 pi();
  static constexpr number_fixed32 e();

  number_fixed32 cos() const;
  number_fixed32 sin() const;

  FORCE_INLINE constexpr number_fixed32 abs() const {
    number_fixed32 l_value = number_fixed32(0);
    l_value.m_value = sys::abs(m_value);
    return l_value;
  };

  constexpr number_fixed32 operator+(number_fixed32 p_other) const;
  constexpr number_fixed32 operator+(number_uint8 p_other) const;
  constexpr number_fixed32 operator+(number_int8 p_other) const;
  constexpr number_fixed32 operator+(number_uint16 p_other) const;
  constexpr number_fixed32 operator+(number_int16 p_other) const;
  constexpr number_fixed32 operator+(number_uint32 p_other) const;
  constexpr number_fixed32 operator+(number_int32 p_other) const;
  constexpr number_fixed32 operator+(number_float32 p_other) const;

  constexpr number_fixed32 &operator+=(number_fixed32 p_other);
  constexpr number_fixed32 &operator+=(number_uint8 p_other);
  constexpr number_fixed32 &operator+=(number_int8 p_other);
  constexpr number_fixed32 &operator+=(number_uint16 p_other);
  constexpr number_fixed32 &operator+=(number_int16 p_other);
  constexpr number_fixed32 &operator+=(number_uint32 p_other);
  constexpr number_fixed32 &operator+=(number_int32 p_other);
  constexpr number_fixed32 &operator+=(number_float32 p_other);

  constexpr number_fixed32 operator-(number_fixed32 p_other) const;
  constexpr number_fixed32 operator-(number_uint8 p_other) const;
  constexpr number_fixed32 operator-(number_int8 p_other) const;
  constexpr number_fixed32 operator-(number_uint16 p_other) const;
  constexpr number_fixed32 operator-(number_int16 p_other) const;
  constexpr number_fixed32 operator-(number_uint32 p_other) const;
  constexpr number_fixed32 operator-(number_int32 p_other) const;
  constexpr number_fixed32 operator-(number_float32 p_other) const;

  constexpr number_fixed32 &operator-=(number_fixed32 p_other);
  constexpr number_fixed32 &operator-=(number_uint8 p_other);
  constexpr number_fixed32 &operator-=(number_int8 p_other);
  constexpr number_fixed32 &operator-=(number_uint16 p_other);
  constexpr number_fixed32 &operator-=(number_int16 p_other);
  constexpr number_fixed32 &operator-=(number_uint32 p_other);
  constexpr number_fixed32 &operator-=(number_int32 p_other);
  constexpr number_fixed32 &operator-=(number_float32 p_other);

  constexpr number_fixed32 operator*(number_fixed32 p_other) const;
  constexpr number_fixed32 operator*(number_uint8 p_other) const;
  constexpr number_fixed32 operator*(number_int8 p_other) const;
  constexpr number_fixed32 operator*(number_uint16 p_other) const;
  constexpr number_fixed32 operator*(number_int16 p_other) const;
  constexpr number_fixed32 operator*(number_uint32 p_other) const;
  constexpr number_fixed32 operator*(number_int32 p_other) const;
  constexpr number_fixed32 operator*(number_float32 p_other) const;

  constexpr number_fixed32 &operator*=(number_fixed32 p_other);

  constexpr number_fixed32 operator/(number_fixed32 p_other) const;

  constexpr number_fixed32 &operator/=(number_fixed32 p_other);

  constexpr ui8 operator==(number_fixed32 p_other) const;
  
  constexpr ui8 operator<(number_fixed32 p_other) const;

  constexpr ui8 operator<=(number_fixed32 p_other) const;

  constexpr ui8 operator>(number_fixed32 p_other) const;

  constexpr ui8 operator>=(number_fixed32 p_other) const;

  constexpr number_fixed32 operator%(number_fixed32 p_other) const;
#endif
};

template <typename T, ui8 ScaleFactor> struct is_fixed<fixed<T, ScaleFactor>> {
  static constexpr ui8 value = 1;
};

template <typename T> constexpr NumberType get_number_type() {
  if constexpr (is_builtin<T>::value && is_builtin_floating<T>::value) {
    return NumberType::Floating;
  } else if constexpr (is_builtin<T>::value && !is_builtin_floating<T>::value) {
    return NumberType::Integer;
  } else if constexpr (is_fixed<T>::value) {
    return NumberType::Fixed;
  }
  return NumberType::Integer;
};

}; // namespace m

using fixed32 = m::fixed<i32, 10>;