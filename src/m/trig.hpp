#pragma once

#include <cor/assertions.hpp>
#include <m/fix.hpp>

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
  if (p_value.m_value == 0) {
    return 0;
  };
  assert_debug(p_value > 0);

  NumberType l_estimate = p_value;
  NumberType l_value =
      (l_estimate / NumberType(2)) + (p_value / (l_estimate * NumberType(2)));
  while ((l_estimate - l_value).abs() > NumberType(1)) {
    l_estimate = l_value;
    l_value =
        (l_estimate / NumberType(2)) + (p_value / (l_estimate * NumberType(2)));
  }
  return l_value;
};

// TODO -> remove this
inline static f32 sqrt(f32 p_value) { return std::sqrt(p_value); };

template <typename NumberType>
inline static NumberType sin_polynomial(NumberType p_angle) {
  NumberType l_angle = p_angle;
  NumberType l_pi_times_2 = NumberType::pi * NumberType(2);
  i8 l_sign = 1;
  l_angle = l_angle % l_pi_times_2; // periodicity
  if (l_angle < 0) {
    l_sign = -1;
    l_angle = l_angle * NumberType(-1);
  }
  // symmetry

  if (l_angle >= NumberType::pi_2) {
    if (l_angle >= NumberType::pi) {
      l_sign *= -1;
      if (l_angle >= (NumberType::pi + NumberType::pi_2)) {
        // 3pi/4 - 2pi
        l_angle -= NumberType::pi;
        l_angle = details::__trig_helper_mirror(l_angle, NumberType::pi_2);
      } else {
        // pi - 3pi/4
        l_angle -= NumberType::pi;
      }
    } else {
      if (l_angle >= NumberType::pi_4) {
        // pi/4 - pi
        l_angle = (NumberType::pi - l_angle);
      }
    }
  }

  NumberType l_angle_3 = l_angle * l_angle * l_angle;
  NumberType l_angle_5 = l_angle_3 * l_angle * l_angle;
  NumberType l_sin = l_angle;
  l_sin -= l_angle_3 / NumberType(6);
  l_sin += l_angle_5 / NumberType(120);

  // TODO -> remove that ? This is here because of rounding errors.
  if (l_sin > NumberType(1)) {
    l_sin = NumberType(1);
  }

  l_sin.m_value *= l_sign;
  return l_sin;
};

// TODO -> remove this
inline static f32 sin(f32 p_angle) { return sys::sin(p_angle); };

template <typename NumberType>
inline static NumberType cos_polynomial(NumberType p_angle) {
  return sin_polynomial(p_angle + NumberType::pi_2);
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
  l_tan += (NumberType(2) * l_angle_5) / NumberType(15);
  l_tan += (NumberType(17) * l_angle_7) / NumberType(315);
  return l_tan;
};

template <typename NumberType>
inline static NumberType __fix_tan_identity(NumberType p_angle) {
  NumberType l_angle_half = p_angle / NumberType(2);
  NumberType l_angle_half_tan = __fix_tan_polynomial(l_angle_half);
  return (NumberType(2) * l_angle_half_tan) /
         (NumberType(1) - (l_angle_half_tan * l_angle_half_tan));
};

}; // namespace details

template <typename NumberType>
inline static NumberType tan_polynomial(NumberType p_angle) {
  NumberType l_angle = p_angle;
  l_angle = l_angle % NumberType::pi; // periodicity
  i8 l_sign = 1;
  if (l_angle < 0) {
    l_sign = -1;
    l_angle.m_value *= -1;
  }

  if (l_angle >= NumberType::pi_2) {
    l_angle = details::__trig_helper_mirror(l_angle, NumberType::pi_2);
    l_sign *= -1;
  }

  // reciprocity
  if (l_angle >= NumberType::pi_4) {
    l_angle = NumberType::pi_2 - l_angle;

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
  l_atan = l_atan - l_length_3 / NumberType(3);
  l_atan = l_atan + l_length_5 / NumberType(5);
  return l_atan;
};

template <typename NumberType>
inline static NumberType __arctan_identitiy(NumberType p_length) {
  const NumberType l_sqrt_3 = sqrt_polynomial(NumberType(3));
  NumberType l_new_length = (l_sqrt_3 * p_length) - NumberType(1);
  l_new_length = l_new_length / (l_sqrt_3 + p_length);
  return (NumberType::pi / NumberType(6)) + __arctan_polynomial(l_new_length);
};

template <typename NumberType>
inline static NumberType __arctan_complement(NumberType p_length) {
  return (NumberType::pi_4 * NumberType(2)) -
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
  if (p_length == NumberType(1)) {
    return NumberType::pi_2;
  }
  if (p_length == NumberType(-1)) {
    return NumberType(-1) * NumberType::pi_2;
  }
  assert_debug(p_length < fix32_one);
  assert_debug(p_length > -1 * fix32_one);
  NumberType l_atan_length =
      p_length / sqrt_polynomial(NumberType(1) - (p_length * p_length));
  return arctan_polynomial(l_atan_length);
};

inline static f32 arcsin(f32 p_length) { return std::asin(p_length); };

template <typename NumberType>
inline static NumberType arccos_polynomial(NumberType p_length) {
  return NumberType::pi_2 - arcsin_polynomial(p_length);
};

// TODO Remove that
inline static f32 arccos(f32 p_length) { return std::acos(p_length); };

}; // namespace m