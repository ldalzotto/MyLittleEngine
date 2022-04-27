#pragma once

#define assert_debug(p_condition) sys::sassert(p_condition)
#define block_debug(p_code) p_code

#include <Eigen/Eigen>

using i8_t = char;
using i16_t = short;
using i32_t = int;
using i64_t = long long;

using ui8_t = unsigned char;
using ui16_t = unsigned short;
using ui32_t = unsigned int;
using ui64_t = unsigned long long;

using uimax_t = ui32_t;

using f32_t = float;

template <typename ValueType> struct number {
  ValueType value;
  number(){};
  explicit number(ValueType p_value) : value(p_value){};

  number operator+(number p_value) { return number(p_value.value + value); };
  number &operator++() {
    value += 1;
    return *this;
  };
  number operator++(int p_value) { return number(value + p_value); };
  number operator-(number p_value) { return number(p_value.value - value); };
  number &operator--() {
    value -= 1;
    return *this;
  };
  number operator--(int p_value) { return number(value - p_value); };
  number operator*(number p_value) { return number(value * p_value.value); };
  number &operator*=(number p_value) {
    value *= p_value.value;
    return *this;
  };
  number operator/(number p_value) { return number(value / p_value.value); };
  number &operator/=(number p_value) {
    value /= p_value.value;
    return *this;
  };

  bool operator==(number p_value) { return value == p_value.value; };
  bool operator!=(number p_value) { return value != p_value.value; };
};

using i8 = number<i8_t>;
using i16 = number<i16_t>;
using i32 = number<i32_t>;
using i64 = number<i64_t>;

using ui8 = number<ui8_t>;
using ui16 = number<ui16_t>;
using ui32 = number<ui32_t>;
using ui64 = number<ui64_t>;
using uimax = number<uimax_t>;

using f32 = number<f32_t>;

template <typename T, int N> using vec = Eigen::Vector<T, N>;
template <typename T, int ROW, int COL> using mat = Eigen::Matrix<T, ROW, COL>;