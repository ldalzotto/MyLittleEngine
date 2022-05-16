#pragma once

#include <cor/assertions.hpp>
#include <m/fix.hpp>

namespace m {

namespace details {
inline static ff32 __trig_helper_mirror(ff32 p_value, ff32 p_mirrored_to) {
  return p_mirrored_to - (p_value - p_mirrored_to);
};
}; // namespace details

inline static ff32 sqrt(ff32 p_value) {
  if (p_value.m_value == 0) {
    return 0;
  };
  assert_debug(p_value > 0);

  ff32 l_estimate = p_value;
  ff32 l_value = (l_estimate / ff32(2)) + (p_value / (l_estimate * ff32(2)));
  while (abs(l_estimate - l_value).m_value > 1) {
    l_estimate = l_value;
    l_value = (l_estimate / ff32(2)) + (p_value / (l_estimate * ff32(2)));
  }
  return l_value;
};

inline static f32 sqrt(f32 p_value) { return std::sqrt(p_value); };

inline static ff32 sin(ff32 p_angle) {
  ff32 l_angle = p_angle;
  ff32 l_pi_times_2 = ff32::pi * ff32(2);
  i8 l_sign = 1;
  l_angle = l_angle % l_pi_times_2; // periodicity
  if (l_angle < 0) {
    l_sign = -1;
    l_angle = l_angle * ff32(-1);
  }
  // symmetry

  if (l_angle >= ff32::pi_2) {
    if (l_angle >= ff32::pi) {
      l_sign *= -1;
      if (l_angle >= (ff32::pi + ff32::pi_2)) {
        // 3pi/4 - 2pi
        l_angle -= ff32::pi;
        l_angle = details::__trig_helper_mirror(l_angle, ff32::pi_2);
      } else {
        // pi - 3pi/4
        l_angle -= ff32::pi;
      }
    } else {
      if (l_angle >= ff32::pi_4) {
        // pi/4 - pi
        l_angle = (ff32::pi - l_angle);
      }
    }
  }

  ff32 l_angle_3 = l_angle * l_angle * l_angle;
  ff32 l_angle_5 = l_angle_3 * l_angle * l_angle;
  ff32 l_sin = l_angle;
  l_sin -= l_angle_3 / ff32(6);
  l_sin += l_angle_5 / ff32(120);

  // TODO -> remove that ? This is here because of rounding errors.
  if (l_sin > ff32::one()) {
    l_sin = ff32::one();
  }

  l_sin.m_value *= l_sign;
  return l_sin;
};

inline static f32 sin(f32 p_angle) { return sys::sin(p_angle); };

inline static ff32 cos(ff32 p_angle) { return sin(p_angle + ff32::pi_2); };
inline static f32 cos(f32 p_angle) { return sys::cos(p_angle); };

namespace details {
inline static ff32 __fix32_tan_polynomial(ff32 p_angle) {
  ff32 l_tan = p_angle;
  ff32 l_angle_3 = p_angle * p_angle * p_angle;
  ff32 l_angle_5 = l_angle_3 * p_angle * p_angle;
  ff32 l_angle_7 = l_angle_5 * p_angle * p_angle;
  l_tan += (l_angle_3 / 3);
  l_tan += (ff32(2) * l_angle_5) / ff32(15);
  l_tan += (ff32(17) * l_angle_7) / ff32(315);
  return l_tan;
};

inline static ff32 __fix32_tan_identity(ff32 p_angle) {
  ff32 l_angle_half = p_angle / ff32(2);
  ff32 l_angle_half_tan = __fix32_tan_polynomial(l_angle_half);
  return (ff32(2) * l_angle_half_tan) /
         (ff32::one() - (l_angle_half_tan * l_angle_half_tan));
};

}; // namespace details

inline static ff32 tan(ff32 p_angle) {
  ff32 l_angle = p_angle;
  l_angle = l_angle % ff32::pi; // periodicity
  i8 l_sign = 1;
  if (l_angle < 0) {
    l_sign = -1;
    l_angle.m_value *= -1;
  }

  if (l_angle >= ff32::pi_2) {
    l_angle = details::__trig_helper_mirror(l_angle, ff32::pi_2);
    l_sign *= -1;
  }

  // reciprocity
  if (l_angle >= ff32::pi_4) {
    l_angle = ff32::pi_2 - l_angle;

    ff32 l_tan = details::__fix32_tan_identity(l_angle);
    return l_tan * l_sign;
  }

  ff32 l_tan = details::__fix32_tan_polynomial(l_angle);
  return l_tan * l_sign;
};

namespace details {

inline static ff32 __fix32_arctan_polynomial(ff32 p_length) {
  ff32 l_length_3 = p_length * p_length * p_length;
  ff32 l_length_5 = l_length_3 * p_length * p_length;
  ff32 l_atan = p_length;
  l_atan = l_atan - l_length_3 / ff32(3);
  l_atan = l_atan + l_length_5 / ff32(5);
  return l_atan;
};

inline static ff32 __fix32_arctan_identitiy(ff32 p_length) {
  const ff32 l_sqrt_3 = sqrt(ff32(3));
  ff32 l_new_length = (l_sqrt_3 * p_length) - ff32::one();
  l_new_length = l_new_length / (l_sqrt_3 + p_length);
  return (ff32::pi / ff32(6)) + __fix32_arctan_polynomial(l_new_length);
};

inline static ff32 __fix32_arctan_complement(ff32 p_length) {
  return (ff32::pi_4 * ff32(2)) -
         __fix32_arctan_polynomial(ff32::one() / p_length);
};

}; // namespace details

inline static ff32 arctan(ff32 p_length) {
  ff32 l_length = p_length;
  i8 l_sign = 1;
  if (l_length < 0) {
    l_length *= -1;
    l_sign *= -1;
  }

  if (l_length > ff32::one()) {
    return ff32(l_sign) * details::__fix32_arctan_complement(l_length);
  }

  const ff32 l_identity_criteria = ff32(2) - sqrt(ff32(3));
  if (l_length <= l_identity_criteria) {
    return ff32(l_sign) * details::__fix32_arctan_identitiy(l_length);
  }

  return ff32(l_sign) * details::__fix32_arctan_polynomial(l_length);
};

inline static f32 arctan(f32 p_length) { return std::atan(p_length); };

inline static ff32 arcsin(ff32 p_length) {
  if (p_length == ff32::one()) {
    return ff32::pi_2;
  }
  if (p_length == ff32::one() * -1) {
    return ff32(-1) * ff32::pi_2;
  }
  assert_debug(p_length < fix32_one);
  assert_debug(p_length > -1 * fix32_one);
  ff32 l_atan_length = p_length / sqrt(ff32::one() - (p_length * p_length));
  return arctan(l_atan_length);
};

inline static f32 arcsin(f32 p_length) { return std::asin(p_length); };

inline static ff32 arccos(ff32 p_length) {
  return ff32::pi_2 - arcsin(p_length);
};

inline static f32 arccos(f32 p_length) { return std::acos(p_length); };

}; // namespace m