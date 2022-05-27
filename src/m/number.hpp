#pragma once

#include <cor/types.hpp>
#include <sys/sys.hpp>

template <typename T> struct number;

#define declare_number_struct(type)                                            \
  template <> struct number<type> {                                            \
    using value_type = type;                                                   \
    value_type m_value;                                                        \
                                                                               \
    FORCE_INLINE number() = default;                                           \
    FORCE_INLINE constexpr number(value_type p_value) : m_value(p_value){};    \
    FORCE_INLINE constexpr value_type &value() { return m_value; };            \
    FORCE_INLINE constexpr const value_type &value() const {                   \
      return m_value;                                                          \
    };                                                                         \
                                                                               \
    template <typename TT>                                                     \
    constexpr number operator+(number<TT> p_other) const;                      \
    template <typename TT> constexpr number &operator+=(number<TT> p_other);   \
    template <typename TT>                                                     \
    constexpr number operator-(number<TT> p_other) const;                      \
    template <typename TT> constexpr number operator-=(number<TT> p_other);    \
    template <typename TT>                                                     \
    constexpr number operator*(number<TT> p_other) const;                      \
    template <typename TT> constexpr number operator*=(number<TT> p_other);    \
    template <typename TT>                                                     \
    constexpr number operator/(number<TT> p_other) const;                      \
    template <typename TT> constexpr number operator/=(number<TT> p_other);    \
    template <typename TT> constexpr ui8 operator==(number<TT> p_other) const; \
    template <typename TT>                                                     \
    constexpr number operator%(number<TT> p_other) const;                      \
    declare_number_struct_trig                                                 \
  }

#define declare_number_struct_trig

declare_number_struct(i8);
declare_number_struct(ui8);
declare_number_struct(i16);
declare_number_struct(ui16);
declare_number_struct(i32);
declare_number_struct(ui32);

#undef declare_number_struct_trig
#define declare_number_struct_trig                                             \
  static const number pi_8;                                                    \
  static const number pi_4;                                                    \
  static const number pi_2;                                                    \
  static const number pi;                                                      \
  static const number e;                                                       \
  number cos() const;                                                          \
  number sin() const;

declare_number_struct(f32);

#undef declare_number_struct_trig
#undef declare_number_struct

#define declare_add_operator(left_type, right_type)                            \
  template <>                                                                  \
  FORCE_INLINE constexpr number<left_type> number<left_type>::operator+        \
      <right_type>(number<right_type> p_other) const {                         \
    return number<left_type>(m_value + p_other.m_value);                       \
  };                                                                           \
  template <>                                                                  \
  FORCE_INLINE constexpr number<left_type> &number<left_type>::operator+=      \
      <right_type>(number<right_type> p_other) {                               \
    m_value += p_other.m_value;                                                \
    return *this;                                                              \
  };

#define declare_sub_operator(left_type, right_type)                            \
  template <>                                                                  \
  FORCE_INLINE constexpr number<left_type> number<left_type>::operator-        \
      <right_type>(number<right_type> p_other) const {                         \
    return number<left_type>(m_value - p_other.m_value);                       \
  };                                                                           \
  template <>                                                                  \
  FORCE_INLINE constexpr number<left_type> number<left_type>::operator-=       \
      <right_type>(number<right_type> p_other) {                               \
    m_value -= p_other.m_value;                                                \
    return *this;                                                              \
  };

#define declare_mult_operator(left_type, right_type)                           \
  template <>                                                                  \
  FORCE_INLINE constexpr number<left_type> number<left_type>::operator*        \
      <right_type>(number<right_type> p_other) const {                         \
    return number<left_type>(m_value * p_other.m_value);                       \
  };                                                                           \
  template <>                                                                  \
  FORCE_INLINE constexpr number<left_type> number<left_type>::operator*=       \
      <right_type>(number<right_type> p_other) {                               \
    m_value *= p_other.m_value;                                                \
    return *this;                                                              \
  };

#define declare_div_operator(left_type, right_type)                            \
  template <>                                                                  \
  FORCE_INLINE constexpr number<left_type> number<left_type>::operator/        \
      <right_type>(number<right_type> p_other) const {                         \
    return number<left_type>(m_value / p_other.m_value);                       \
  };                                                                           \
  template <>                                                                  \
  FORCE_INLINE constexpr number<left_type> number<left_type>::operator/=       \
      <right_type>(number<right_type> p_other) {                               \
    m_value /= p_other.m_value;                                                \
    return *this;                                                              \
  };

#define declare_eq_operator(left_type, right_type)                             \
  template <>                                                                  \
  FORCE_INLINE constexpr ui8 number<left_type>::operator==                     \
      <right_type>(number<right_type> p_other) const {                         \
    return m_value == p_other.m_value;                                         \
  };

#define declare_mod_operator(left_type, right_type)                            \
  template <>                                                                  \
  FORCE_INLINE constexpr number<left_type> number<left_type>::operator%        \
      <right_type>(number<right_type> p_other) const {                         \
    return number<left_type>(m_value % p_other.m_value);                       \
  };

#define declare_trig_constants(number_type)                                    \
  constexpr number number<number_type>::pi_8 =                                 \
      number<number_type>(3.14159265358979323846 / 8);                         \
  constexpr number number<number_type>::pi_4 =                                 \
      number<number_type>::pi_8 * number<number_type>(2);                      \
  constexpr number number<number_type>::pi_2 =                                 \
      number<number_type>::pi_8 * number<number_type>(4);                      \
  constexpr number number<number_type>::pi =                                   \
      number<number_type>::pi_8 * number<number_type>(8);                      \
  constexpr number number<number_type>::e =                                    \
      number<number_type>(2.7182818284590);

#define declare_trig_operations(number_type)                                   \
  FORCE_INLINE number<number_type> number<number_type>::cos() const {          \
    return sys::cos(m_value);                                                  \
  };                                                                           \
  FORCE_INLINE number<number_type> number<number_type>::sin() const {          \
    return sys::sin(m_value);                                                  \
  };

#define i8_additive_operations                                                 \
  X(i8, ui8)                                                                   \
  X(i8, i8)

#define i8_multiplicative_operations i8_additive_operations

#define ui8_additive_operations                                                \
  X(ui8, ui8)                                                                  \
  X(ui8, i8)

#define ui8_multiplicative_operations X(ui8, ui8)

#define i16_additive_operations                                                \
  X(i16, ui8)                                                                  \
  X(i16, i8)                                                                   \
  X(i16, ui16)                                                                 \
  X(i16, i16)

#define i16_multiplicative_operations i16_additive_operations

#define ui16_additive_operations                                               \
  X(ui16, ui8)                                                                 \
  X(ui16, i8)                                                                  \
  X(ui16, ui16)                                                                \
  X(ui16, i16)

#define ui16_multiplicative_operations                                         \
  X(ui16, ui8)                                                                 \
  X(ui16, ui16)

#define i32_additive_operations                                                \
  X(i32, ui8)                                                                  \
  X(i32, ui16)                                                                 \
  X(i32, ui32)                                                                 \
  X(i32, i8)                                                                   \
  X(i32, i16)                                                                  \
  X(i32, i32)

#define i32_multiplicative_operations i32_additive_operations

#define ui32_additive_operations                                               \
  X(ui32, ui8)                                                                 \
  X(ui32, ui16)                                                                \
  X(ui32, ui32)                                                                \
  X(ui32, i8)                                                                  \
  X(ui32, i16)                                                                 \
  X(ui32, i32)

#define ui32_multiplicative_operations                                         \
  X(ui32, ui8)                                                                 \
  X(ui32, ui16)                                                                \
  X(ui32, ui32)

#define f32_additive_operations                                                \
  X(f32, ui8)                                                                  \
  X(f32, ui16)                                                                 \
  X(f32, ui32)                                                                 \
  X(f32, i8)                                                                   \
  X(f32, i16)                                                                  \
  X(f32, i32)                                                                  \
  X(f32, f32)

#define f32_eq_operations X(f32, f32)

#define f32_multiplicative_operations f32_additive_operations

#define X declare_add_operator
i8_additive_operations;
#undef X
#define X declare_sub_operator
i8_additive_operations;
#undef X
#define X declare_mult_operator
i8_multiplicative_operations;
#undef X
#define X declare_div_operator
i8_multiplicative_operations;
#undef X
#define X declare_eq_operator
i8_additive_operations;
#undef X
#define X declare_mod_operator
i8_multiplicative_operations;
#undef X

#define X declare_add_operator
ui8_additive_operations;
#undef X
#define X declare_sub_operator
ui8_additive_operations;
#undef X
#define X declare_mult_operator
ui8_multiplicative_operations;
#undef X
#define X declare_div_operator
ui8_multiplicative_operations;
#undef X
#define X declare_eq_operator
ui8_additive_operations;
#undef X
#define X declare_mod_operator
ui8_multiplicative_operations;
#undef X

#define X declare_add_operator
ui16_additive_operations;
#undef X
#define X declare_sub_operator
ui16_additive_operations;
#undef X
#define X declare_mult_operator
ui16_multiplicative_operations;
#undef X
#define X declare_div_operator
ui16_multiplicative_operations;
#undef X
#define X declare_eq_operator
ui16_additive_operations;
#undef X
#define X declare_mod_operator
ui16_multiplicative_operations;
#undef X

#define X declare_add_operator
i16_additive_operations;
#undef X
#define X declare_sub_operator
i16_additive_operations;
#undef X
#define X declare_mult_operator
i16_multiplicative_operations;
#undef X
#define X declare_div_operator
i16_multiplicative_operations;
#undef X
#define X declare_eq_operator
i16_additive_operations;
#undef X
#define X declare_mod_operator
i16_multiplicative_operations;
#undef X

#define X declare_add_operator
i32_additive_operations;
#undef X
#define X declare_sub_operator
i32_additive_operations;
#undef X
#define X declare_mult_operator
i32_multiplicative_operations;
#undef X
#define X declare_div_operator
i32_multiplicative_operations;
#undef X
#define X declare_eq_operator
i32_additive_operations;
#undef X
#define X declare_mod_operator
i32_multiplicative_operations;
#undef X

#define X declare_add_operator
ui32_additive_operations;
#undef X
#define X declare_sub_operator
ui32_additive_operations;
#undef X
#define X declare_mult_operator
ui32_multiplicative_operations;
#undef X
#define X declare_div_operator
ui32_multiplicative_operations;
#undef X
#define X declare_eq_operator
ui32_additive_operations;
#undef X
#define X declare_mod_operator
ui32_multiplicative_operations;
#undef X

#define X declare_add_operator
f32_additive_operations;
#undef X
#define X declare_sub_operator
f32_additive_operations;
#undef X
#define X declare_mult_operator
f32_multiplicative_operations;
#undef X
#define X declare_div_operator
f32_multiplicative_operations;
#undef X
#define X declare_eq_operator
f32_eq_operations;
#undef X
declare_trig_operations(f32);
declare_trig_constants(f32);

#undef ui8_additive_operations
#undef ui8_multiplicative_operations
#undef i8_additive_operations
#undef i8_multiplicative_operations
#undef i16_additive_operations
#undef i16_multiplicative_operations
#undef ui16_additive_operations
#undef ui16_multiplicative_operations
#undef ui32_additive_operations
#undef ui32_multiplicative_operations
#undef i32_additive_operations
#undef i32_multiplicative_operations
#undef f32_additive_operations
#undef f32_multiplicative_operations
#undef f32_eq_operations

#undef declare_add_operator
#undef declare_sub_operator
#undef declare_mult_operator
#undef declare_div_operator
#undef declare_eq_operator
#undef declare_mod_operator
#undef declare_trig_operations
#undef declare_trig_constants

using int8 = number<i8>;
using int16 = number<i16>;
using int32 = number<i32>;
using uint8 = number<ui8>;
using uint16 = number<ui16>;
using uint32 = number<ui32>;

using intmax = int32;
using uintmax = uint32;

using float32 = number<f32>;