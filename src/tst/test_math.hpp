#pragma once

#include <m/vec.hpp>
namespace {
struct math_tests {
  static void tst() {
    m::vec<ui32, 3> l_1 = {1, 0, 0};
    m::vec<ui32, 3> l_2 = {0, 1, 0};
    l_1 = m::cross(l_1, l_2);
    l_1 += l_1;
  };
};
} // namespace

inline static void test_math() { math_tests::tst(); };