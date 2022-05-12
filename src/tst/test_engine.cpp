#include <doctest.h>

#include <eng/engine.hpp>

TEST_CASE("engine.window.draw") {
  eng::engine l_engine;
  l_engine.allocate();
  l_engine.window_open(800, 800);
  while (l_engine.update()) {
  }

  l_engine.free();
}