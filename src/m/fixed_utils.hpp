#pragma once

#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace m {
struct fixed_utils {
  template <typename Left, typename Right>
  FORCE_INLINE constexpr static void make_floating_builtin(Left &thiz,
                                                           Right p_value) {
    thiz.m_value = sys::nearest(p_value * Left::scale);
  };

  template <typename Left, typename Right>
  FORCE_INLINE constexpr static void make_integer_builtin(Left &thiz,
                                                          Right p_value) {
    thiz.m_value = i32(p_value) << Left::scale_factor;
  };

  template <typename Left, typename Right>
  FORCE_INLINE constexpr static void make_floating_number(Left &thiz,
                                                          Right p_value) {
    thiz.m_value = sys::nearest(p_value.m_value * Left::scale);
  };

  template <typename Left, typename Right>
  FORCE_INLINE constexpr static void make_integer_number(Left &thiz,
                                                         Right p_value) {
    thiz.m_value = i32(p_value.m_value) << Left::scale_factor;
  };

  template <typename Left>
  FORCE_INLINE constexpr static int mult_fixed(const Left &left,
                                               const Left &right) {
    return (((long long)left.m_value * right.m_value) + (Left::scale >> 1)) >>
           Left::scale_factor;
  };

  template <typename Left, typename Right>
  FORCE_INLINE constexpr static int mult_number(const Left &left,
                                                const Right &right) {
    return mult_fixed(left, Left(right));
  };

  template <typename Left>
  FORCE_INLINE constexpr static int div_fixed(const Left &left,
                                              const Left &right) {
    return (((((long long)left.m_value) * Left::scale) + (Left::scale >> 1))) /
           right.m_value;
  };

  template <typename Left, typename Right>
  FORCE_INLINE constexpr static int div_number(const Left &left,
                                                const Right &right) {
    return div_fixed(left, Left(right));
  };
};
}; // namespace m