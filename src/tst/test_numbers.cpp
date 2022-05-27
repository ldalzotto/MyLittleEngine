
#include <doctest.h>

#include <cor/container.hpp>
#include <m/fix.hpp>
#include <m/number.hpp>
#include <m/trig.hpp>

TEST_CASE("math.numbers") {
  REQUIRE((int8(-10) + int8(-10)) == int8(-20));
  REQUIRE((uint8(20) + int8(-20)) == int8(0));

  {
    uint8 l_value = 10;
    l_value += int8(10);
    REQUIRE(l_value == uint8(20));
    l_value -= int8(10);
    REQUIRE(l_value == uint8(10));
    l_value *= uint8(2);
    REQUIRE(l_value == uint8(20));
    l_value /= uint8(2);
    REQUIRE(l_value == uint8(10));
  }

  { constexpr int8 l_value = (int8(10) + int8(20)) % uint8(2); }

  REQUIRE((int8(-20) / uint8(10)) == int8(-2));

  REQUIRE((float32(2.5f) * uint8(2)) == float32(5.0f));
  REQUIRE(float32::pi().cos() == float32(-1));
};

TEST_CASE("math.fixed") {
  ff32 l_value = 10.5;
  f32 l_value_f = (l_value * 2).to_f32();
  REQUIRE(((l_value_f >= 20.9f) && (l_value_f <= 21.1f)));
};

struct trigo_test_accumulator {
  container::span<ff32> m_errors;
  void allocate(uimax p_error_count) { m_errors.allocate(p_error_count); };
  void free() { m_errors.free(); };

  ff32 get_accumulated() {
    ff32 l_accumulated = 0;
    uimax l_error_it;
    for (l_error_it = 0; l_error_it < m_errors.count(); ++l_error_it) {
      l_accumulated += m_errors.at(l_error_it);
    }
    return l_accumulated;
  };

  ff32 get_average() {
    ff32 l_average = get_accumulated();
    l_average = l_average / m_errors.count();
    return l_average;
  };
};

void math_cos_test(ff32 p_angle, trigo_test_accumulator &p_accs,
                   trigo_test_accumulator &p_arc_accs, uimax p_acc_index) {
  ff32 l_cos = m::cos_polynomial(p_angle);
  f32 l_cos_real = m::cos(p_angle.to_f32());
  p_accs.m_errors.at(p_acc_index) = (ff32(l_cos_real) - l_cos).abs();

  ff32 l_arc_cos = m::arccos_polynomial(l_cos);
  f32 l_arc_cos_real = m::arccos(l_cos.to_f32());
  p_arc_accs.m_errors.at(p_acc_index) =
      (ff32(l_arc_cos_real) - l_arc_cos).abs();
};

void math_sin_test(ff32 p_angle, trigo_test_accumulator &p_accs,
                   trigo_test_accumulator &p_arc_accs, uimax p_acc_index) {
  ff32 l_sin = m::sin_polynomial(p_angle);
  f32 l_sin_real = m::sin(p_angle.to_f32());
  ff32 l_sin_real_fixed = ff32(l_sin_real);
  ff32 l_diff = (l_sin_real_fixed - l_sin).abs();
  p_accs.m_errors.at(p_acc_index) = l_diff;

  ff32 l_arc_sin = m::arcsin_polynomial(l_sin);
  f32 l_arc_sin_real = m::arcsin(l_sin.to_f32());
  p_arc_accs.m_errors.at(p_acc_index) =
      (ff32(l_arc_sin_real) - l_arc_sin).abs();
};

void math_tan_test(ff32 p_angle, trigo_test_accumulator &p_accs,
                   trigo_test_accumulator &p_arc_accs, uimax p_acc_index) {
  ff32 l_tan = m::tan_polynomial(p_angle);
  f32 l_tan_real = sys::tan(p_angle.to_f32());
  if (p_angle % ff32::pi_2() == 0) {
    p_accs.m_errors.at(p_acc_index) = 0;
  } else {
    p_accs.m_errors.at(p_acc_index) = (ff32(l_tan_real) - l_tan).abs();
  }

  ff32 l_arc_tan = m::arctan_polynomial(l_tan);
  f32 l_arc_tan_real = m::arctan(l_tan.to_f32());
  p_arc_accs.m_errors.at(p_acc_index) =
      (ff32(l_arc_tan_real) - l_arc_tan).abs();
};

TEST_CASE("math.fixed.sin_cos_tan") {
  container::arr<ff32, 4 * 8> l_cos_angles = {
      ff32::pi_4() * 1,   ff32::pi_4() * 2,   ff32::pi_4() * 3,
      ff32::pi_4() * 4,   ff32::pi_4() * 5,   ff32::pi_4() * 6,
      ff32::pi_4() * 7,   ff32::pi_4() * 8,   ff32::pi_4() * 9,
      ff32::pi_4() * 10,  ff32::pi_4() * 11,  ff32::pi_4() * 12,
      ff32::pi_4() * 13,  ff32::pi_4() * 14,  ff32::pi_4() * 15,
      ff32::pi_4() * 16,  ff32::pi_4() * -1,  ff32::pi_4() * -2,
      ff32::pi_4() * -3,  ff32::pi_4() * -4,  ff32::pi_4() * -5,
      ff32::pi_4() * -6,  ff32::pi_4() * -7,  ff32::pi_4() * -8,
      ff32::pi_4() * -9,  ff32::pi_4() * -10, ff32::pi_4() * -11,
      ff32::pi_4() * -12, ff32::pi_4() * -13, ff32::pi_4() * -14,
      ff32::pi_4() * -15, ff32::pi_4() * -16};

  container::arr<trigo_test_accumulator, 6> l_err_accumulators;
  for (auto l_acc_iter = 0; l_acc_iter < l_err_accumulators.count();
       ++l_acc_iter) {
    l_err_accumulators.at(l_acc_iter).allocate(l_cos_angles.count());
  }

  for (auto l_sin_iter = 0; l_sin_iter < l_cos_angles.count(); ++l_sin_iter) {
    math_sin_test(l_cos_angles.at(l_sin_iter), l_err_accumulators.at(0),
                  l_err_accumulators.at(1), l_sin_iter);
    math_cos_test(l_cos_angles.at(l_sin_iter), l_err_accumulators.at(2),
                  l_err_accumulators.at(3), l_sin_iter);
    math_tan_test(l_cos_angles.at(l_sin_iter), l_err_accumulators.at(4),
                  l_err_accumulators.at(5), l_sin_iter);
  }

  ff32 l_sin_error = l_err_accumulators.at(0).get_average();
  ff32 l_arcsin_error = l_err_accumulators.at(1).get_average();
  ff32 l_cos_error = l_err_accumulators.at(2).get_average();
  ff32 l_arccos_error = l_err_accumulators.at(3).get_average();
  ff32 l_tan_error = l_err_accumulators.at(4).get_average();
  ff32 l_arctan_error = l_err_accumulators.at(5).get_average();

  for (auto l_acc_iter = 0; l_acc_iter < l_err_accumulators.count();
       ++l_acc_iter) {
    l_err_accumulators.at(l_acc_iter).free();
  }

  REQUIRE(l_sin_error.m_value == 1);
  REQUIRE(l_arcsin_error.m_value == 41);
  REQUIRE(l_cos_error.m_value == 1);
  REQUIRE(l_arccos_error.m_value == 41);
  REQUIRE(l_tan_error.m_value == 2);
  REQUIRE(l_arctan_error.m_value == 42);
};

TEST_CASE("math.fixed.sqrt") {
  container::span<ff32> l_sqrt_inputs;
  l_sqrt_inputs.allocate(1000);

  ff32 l_value_delta = ff32(0.5f);
  for (auto i = 0; i < l_sqrt_inputs.count(); ++i) {
    l_sqrt_inputs.at(i) = l_value_delta * ff32(i);
  }

  trigo_test_accumulator l_acc;
  l_acc.allocate(l_sqrt_inputs.count());
  for (auto i = 0; i < l_sqrt_inputs.count(); ++i) {
    ff32 l_sqrt = m::sqrt_polynomial(l_sqrt_inputs.at(i));
    f32 l_real_sqrtf = m::sqrt(l_sqrt_inputs.at(i).to_f32());
    ff32 l_real_sqrtf_fix = ff32(l_real_sqrtf);
    l_acc.m_errors.at(i) = (l_sqrt - l_real_sqrtf_fix).abs();
  }
  ff32 l_sqrt_error = l_acc.get_accumulated();
  REQUIRE(l_sqrt_error.m_value == 699);
  l_acc.free();

  l_sqrt_inputs.free();
};

#include <sys/sys_impl.hpp>