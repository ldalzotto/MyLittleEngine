#pragma once

#include <Eigen/Eigen>

using i8_t = char;
using i16_t = short;
using i32_t = long;
using i64_t = long long;

using ui8_t = unsigned char;
using ui16_t = unsigned short;
using ui32_t = unsigned long;
using ui64_t = unsigned long long;

template <typename ValueType> struct number {
  ValueType value;
  number() {};
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
  operator bool() const { return value != 0; };
};

using i8 = number<i8_t>;
using i16 = number<i16_t>;
using i32 = number<i32_t>;
using i64 = number<i64_t>;

using ui8 = number<ui8_t>;
using ui16 = number<ui16_t>;
using ui32 = number<ui32_t>;
using ui64 = number<ui64_t>;

template <typename T, int N> using vec = Eigen::Vector<T, N>;

#define BOOST_NO_EXCEPTIONS
#include <boost/container/vector.hpp>

namespace cont = boost::container;