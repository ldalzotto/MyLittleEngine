
#include <doctest.h>

#include <cor/container.hpp>
#include <m/math.hpp>
#include <m/trig.hpp>

TEST_CASE("math.fixed") {
  fix32 l_value = 10.5;
  fix32 l_value_f = l_value * 2;
  REQUIRE(((l_value_f >= 20.9f) && (l_value_f <= 21.1f)));
  fix32 l_value_div = l_value / 2;
  REQUIRE(((l_value_div >= fix32(5.20f)) && (l_value_div <= 5.30f)));
};
#if 1

struct trigo_test_accumulator {
  container::span<fix32> m_errors;
  void allocate(uimax p_error_count) { m_errors.allocate(p_error_count); };
  void free() { m_errors.free(); };

  fix32 get_accumulated() {
    fix32 l_accumulated = 0;
    uimax l_error_it;
    for (l_error_it = 0; l_error_it < m_errors.count(); ++l_error_it) {
      l_accumulated += m_errors.at(l_error_it);
    }
    return l_accumulated;
  };

  fix32 get_average() {
    fix32 l_average = get_accumulated();
    l_average = l_average / m_errors.count();
    return l_average;
  };
};

void math_cos_test(fix32 p_angle, trigo_test_accumulator &p_accs,
                   trigo_test_accumulator &p_arc_accs, uimax p_acc_index) {
  fix32 l_cos = m::cos_polynomial(p_angle);
  f32 l_cos_real = m::cos(f32(p_angle));
  p_accs.m_errors.at(p_acc_index) = m::abs(fix32(l_cos_real) - l_cos);

  fix32 l_arc_cos = m::arccos_polynomial(l_cos);
  f32 l_arc_cos_real = m::arccos(f32(l_cos));
  p_arc_accs.m_errors.at(p_acc_index) =
      m::abs(fix32(l_arc_cos_real) - l_arc_cos);
};

void math_sin_test(fix32 p_angle, trigo_test_accumulator &p_accs,
                   trigo_test_accumulator &p_arc_accs, uimax p_acc_index) {
  fix32 l_sin = m::sin_polynomial(p_angle);
  f32 l_sin_real = m::sin(f32(p_angle));
  fix32 l_sin_real_fixed = l_sin_real;
  fix32 l_diff = m::abs(l_sin_real_fixed - l_sin);
  p_accs.m_errors.at(p_acc_index) = l_diff;

  fix32 l_arc_sin = m::arcsin_polynomial(l_sin);
  f32 l_arc_sin_real = m::arcsin(f32(l_sin));
  p_arc_accs.m_errors.at(p_acc_index) =
      m::abs(fix32(l_arc_sin_real) - l_arc_sin);
};

void math_tan_test(fix32 p_angle, trigo_test_accumulator &p_accs,
                   trigo_test_accumulator &p_arc_accs, uimax p_acc_index) {
  fix32 l_tan = m::tan_polynomial(p_angle);
  f32 l_tan_real = sys::tan(f32(p_angle));
  if (p_angle % m::pi_2<fix32>() == 0) {
    p_accs.m_errors.at(p_acc_index) = 0;
  } else {
    p_accs.m_errors.at(p_acc_index) = m::abs(fix32(l_tan_real) - l_tan);
  }

  fix32 l_arc_tan = m::arctan_polynomial(l_tan);
  f32 l_arc_tan_real = m::arctan(f32(l_tan));
  p_arc_accs.m_errors.at(p_acc_index) =
      m::abs(fix32(l_arc_tan_real) - l_arc_tan);
};

TEST_CASE("math.fixed.sin_cos_tan") {
  container::arr<fix32, 4 * 8> l_cos_angles = {
      m::pi_4<fix32>() * 1,   m::pi_4<fix32>() * 2,
      m::pi_4<fix32>() * 3,   m::pi_4<fix32>() * 4,
      m::pi_4<fix32>() * 5,   m::pi_4<fix32>() * 6,
      m::pi_4<fix32>() * 7,   m::pi_4<fix32>() * 8,
      m::pi_4<fix32>() * 9,   m::pi_4<fix32>() * 10,
      m::pi_4<fix32>() * 11,  m::pi_4<fix32>() * 12,
      m::pi_4<fix32>() * 13,  m::pi_4<fix32>() * 14,
      m::pi_4<fix32>() * 15,  m::pi_4<fix32>() * 16,
      m::pi_4<fix32>() * -1,  m::pi_4<fix32>() * -2,
      m::pi_4<fix32>() * -3,  m::pi_4<fix32>() * -4,
      m::pi_4<fix32>() * -5,  m::pi_4<fix32>() * -6,
      m::pi_4<fix32>() * -7,  m::pi_4<fix32>() * -8,
      m::pi_4<fix32>() * -9,  m::pi_4<fix32>() * -10,
      m::pi_4<fix32>() * -11, m::pi_4<fix32>() * -12,
      m::pi_4<fix32>() * -13, m::pi_4<fix32>() * -14,
      m::pi_4<fix32>() * -15, m::pi_4<fix32>() * -16};

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

  fix32 l_sin_error = l_err_accumulators.at(0).get_average();
  fix32 l_arcsin_error = l_err_accumulators.at(1).get_average();
  fix32 l_cos_error = l_err_accumulators.at(2).get_average();
  fix32 l_arccos_error = l_err_accumulators.at(3).get_average();
  fix32 l_tan_error = l_err_accumulators.at(4).get_average();
  fix32 l_arctan_error = l_err_accumulators.at(5).get_average();

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
  container::span<fix32> l_sqrt_inputs;
  l_sqrt_inputs.allocate(1000);

  fix32 l_value_delta = fix32(0.5f);
  for (auto i = 0; i < l_sqrt_inputs.count(); ++i) {
    l_sqrt_inputs.at(i) = l_value_delta * fix32(i);
  }

  trigo_test_accumulator l_acc;
  l_acc.allocate(l_sqrt_inputs.count());
  for (auto i = 0; i < l_sqrt_inputs.count(); ++i) {
    fix32 l_sqrt = m::sqrt_polynomial(l_sqrt_inputs.at(i));
    f32 l_real_sqrtf = m::sqrt(f32(l_sqrt_inputs.at(i)));
    fix32 l_real_sqrtf_fix = fix32(l_real_sqrtf);
    l_acc.m_errors.at(i) = m::abs(l_sqrt - l_real_sqrtf_fix);
  }
  fix32 l_sqrt_error = l_acc.get_accumulated();
  REQUIRE(l_sqrt_error.m_value == 699);
  l_acc.free();

  l_sqrt_inputs.free();
};

#endif

#include <sys/sys_impl.hpp>