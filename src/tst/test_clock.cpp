#include <doctest.h>
#include <m/math.hpp>
#include <sys/clock.hpp>
#include <sys/clock_impl.hpp>

TEST_CASE("clock") {
  struct clock l_clock;
  auto l_second_offset = 90000;
  auto l_delta_ms = 60;
  auto l_delta_s = fix32(l_delta_ms) / (1000);
  s_next_clock_time = clock_time::make_s_ms(l_second_offset, 0);
  l_clock.init(clock_time::make_s_ms(0, l_delta_ms));
  s_next_clock_time = clock_time::make_s_ms(l_second_offset, 5);
  REQUIRE(!l_clock.update());
  s_next_clock_time = clock_time::make_s_ms(l_second_offset, 10);
  REQUIRE(!l_clock.update());
  s_next_clock_time = clock_time::make_s_ms(l_second_offset, 59);
  REQUIRE(!l_clock.update());
  s_next_clock_time = clock_time::make_s_ms(l_second_offset, 60);
  REQUIRE(l_clock.update()); // elapsed time is > to delta
  REQUIRE(l_clock.delta() == l_delta_s);
  REQUIRE(!l_clock.update()); // further call doesn't do anything

  // Making sure that the clock works when the internal second value increments
  s_next_clock_time = clock_time::make_s_mics(0, 999999);
  l_clock.init(clock_time::make_s_ms(0, l_delta_ms));
  s_next_clock_time = clock_time::make_s_mics(1, 0);
  REQUIRE(!l_clock.update());
  s_next_clock_time = clock_time::make_s_mics(1, 59998);
  REQUIRE(!l_clock.update());
  s_next_clock_time = clock_time::make_s_mics(1, 59999);
  REQUIRE(l_clock.update());
  REQUIRE(l_clock.delta() == l_delta_s);

  // When the delta ms is elapsed multiple time.
  // Then only one huge step is returned
  s_next_clock_time = clock_time::make_s_mics(0, 0);
  l_clock.init(clock_time::make_s_ms(0, l_delta_ms));
  REQUIRE(!l_clock.update());
  s_next_clock_time = clock_time::make_s_ms(0, l_delta_ms * 2.5);
  REQUIRE(l_clock.update());
  REQUIRE(l_clock.delta() == l_delta_s * 2);
};

#include <sys/sys_impl.hpp>