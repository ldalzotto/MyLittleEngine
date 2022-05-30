#pragma once

#include <cmath>
#include <cor/assertions.hpp>
#include <m/math.hpp>

// TODO -> remove NumberType(1) that is being used to properly convert type.

namespace m {

namespace details {
template <typename NumberType>
inline static NumberType __trig_helper_mirror(NumberType p_value,
                                              NumberType p_mirrored_to) {
  return p_mirrored_to - (p_value - p_mirrored_to);
};
}; // namespace details

template <typename NumberType>
inline static NumberType sqrt_polynomial(NumberType p_value) {
  if (p_value == 0) {
    return 0;
  };
  assert_debug(p_value > 0);

  NumberType l_estimate = p_value;
  NumberType l_value = (l_estimate / 2) + (p_value / (l_estimate * 2));
  while (m::abs(l_estimate - l_value) > m::epsilon<NumberType>::value()) {
    l_estimate = l_value;
    l_value = (l_estimate / 2) + (p_value / (l_estimate * 2));
  }
  return l_value;
};

// TODO -> remove this
inline static f32 sqrt(f32 p_value) { return std::sqrt(p_value); };

template <typename NumberType>
inline static NumberType sin_polynomial(NumberType p_angle) {
  NumberType l_angle = p_angle;
  NumberType l_pi_times_2 = m::pi<NumberType>() * 2;
  i8 l_sign = 1;
  l_angle = l_angle % l_pi_times_2; // periodicity
  if (l_angle < 0) {
    l_sign = -1;
    l_angle = l_angle * -1;
  }
  // symmetry

  if (l_angle >= m::pi_2<NumberType>()) {
    if (l_angle >= m::pi<NumberType>()) {
      l_sign *= -1;
      if (l_angle >= (m::pi<NumberType>() + m::pi_2<NumberType>())) {
        // 3pi/4 - 2pi
        l_angle -= m::pi<NumberType>();
        l_angle = details::__trig_helper_mirror(l_angle, m::pi_2<NumberType>());
      } else {
        // pi - 3pi/4
        l_angle -= m::pi<NumberType>();
      }
    } else {
      if (l_angle >= m::pi_4<NumberType>()) {
        // pi/4 - pi
        l_angle = (m::pi<NumberType>() - l_angle);
      }
    }
  }

  NumberType l_angle_3 = l_angle * l_angle * l_angle;
  NumberType l_angle_5 = l_angle_3 * l_angle * l_angle;
  NumberType l_sin = l_angle;
  l_sin -= l_angle_3 / 6;
  l_sin += l_angle_5 / 120;

  // TODO -> remove that ? This is here because of rounding errors.
  if (l_sin > 1) {
    l_sin = 1;
  }

  l_sin *= l_sign;
  return l_sin;
};

// TODO -> remove this
inline static f32 sin(f32 p_angle) { return sys::sin(p_angle); };

template <typename NumberType>
inline static NumberType cos_polynomial(NumberType p_angle) {
  return sin_polynomial(p_angle + m::pi_2<NumberType>());
};

// TODO -> remove this
inline static f32 cos(f32 p_angle) { return sys::cos(p_angle); };

namespace details {
template <typename NumberType>
inline static NumberType __fix_tan_polynomial(NumberType p_angle) {
  NumberType l_tan = p_angle;
  NumberType l_angle_3 = p_angle * p_angle * p_angle;
  NumberType l_angle_5 = l_angle_3 * p_angle * p_angle;
  NumberType l_angle_7 = l_angle_5 * p_angle * p_angle;
  l_tan += (l_angle_3 / 3);
  l_tan += (NumberType(2) * l_angle_5) / 15;
  l_tan += (NumberType(17) * l_angle_7) / 315;
  return l_tan;
};

template <typename NumberType>
inline static NumberType __fix_tan_identity(NumberType p_angle) {
  NumberType l_angle_half = p_angle / 2;
  NumberType l_angle_half_tan = __fix_tan_polynomial(l_angle_half);
  return (NumberType(2) * l_angle_half_tan) /
         (NumberType(1) - (l_angle_half_tan * l_angle_half_tan));
};

}; // namespace details

template <typename NumberType>
inline static NumberType tan_polynomial(NumberType p_angle) {
  NumberType l_angle = p_angle;
  l_angle = l_angle % m::pi<NumberType>(); // periodicity
  i8 l_sign = 1;
  if (l_angle < 0) {
    l_sign = -1;
    l_angle *= -1;
  }

  if (l_angle >= m::pi_2<NumberType>()) {
    l_angle = details::__trig_helper_mirror(l_angle, m::pi_2<NumberType>());
    l_sign *= -1;
  }

  // reciprocity
  if (l_angle >= m::pi_4<NumberType>()) {
    l_angle = m::pi_2<NumberType>() - l_angle;

    NumberType l_tan = details::__fix_tan_identity(l_angle);
    return l_tan * l_sign;
  }

  NumberType l_tan = details::__fix_tan_polynomial(l_angle);
  return l_tan * l_sign;
};

namespace details {

template <typename NumberType>
inline static NumberType __arctan_polynomial(NumberType p_length) {
  NumberType l_length_3 = p_length * p_length * p_length;
  NumberType l_length_5 = l_length_3 * p_length * p_length;
  NumberType l_atan = p_length;
  l_atan = l_atan - l_length_3 / 3;
  l_atan = l_atan + l_length_5 / 5;
  return l_atan;
};

template <typename NumberType>
inline static NumberType __arctan_identitiy(NumberType p_length) {
  const NumberType l_sqrt_3 = sqrt_polynomial(NumberType(3));
  NumberType l_new_length = (l_sqrt_3 * p_length) - 1;
  l_new_length = l_new_length / (l_sqrt_3 + p_length);
  return (m::pi<NumberType>() / 6) + __arctan_polynomial(l_new_length);
};

template <typename NumberType>
inline static NumberType __arctan_complement(NumberType p_length) {
  return (m::pi_4<NumberType>() * 2) -
         __arctan_polynomial(NumberType(1) / p_length);
};

}; // namespace details

template <typename NumberType>
inline static NumberType arctan_polynomial(NumberType p_length) {
  NumberType l_length = p_length;
  i8 l_sign = 1;
  if (l_length < 0) {
    l_length *= -1;
    l_sign *= -1;
  }

  if (l_length > NumberType(1)) {
    return NumberType(l_sign) * details::__arctan_complement(l_length);
  }

  const NumberType l_identity_criteria =
      NumberType(2) - sqrt_polynomial(NumberType(3));
  if (l_length <= l_identity_criteria) {
    return NumberType(l_sign) * details::__arctan_identitiy(l_length);
  }

  return NumberType(l_sign) * details::__arctan_polynomial(l_length);
};

// TODO Remove this
inline static f32 arctan(f32 p_length) { return std::atan(p_length); };

template <typename NumberType>
inline static NumberType arcsin_polynomial(NumberType p_length) {
  if (p_length == 1) {
    return m::pi_2<NumberType>();
  }
  if (p_length == -1) {
    return NumberType(-1) * m::pi_2<NumberType>();
  }
  assert_debug(p_length < 1);
  assert_debug(p_length > -1);
  NumberType l_atan_length =
      p_length / sqrt_polynomial(NumberType(1) - (p_length * p_length));
  return arctan_polynomial(l_atan_length);
};

inline static f32 arcsin(f32 p_length) { return std::asin(p_length); };

template <typename NumberType>
inline static NumberType arccos_polynomial(NumberType p_length) {
  return m::pi_2<NumberType>() - arcsin_polynomial(p_length);
};

// TODO Remove that
inline static f32 arccos(f32 p_length) { return std::acos(p_length); };

}; // namespace m