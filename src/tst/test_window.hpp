#pragma once

#include <eng/engine.hpp>
#include <sys/win.hpp>

namespace {
struct window_tests {

  inline static void nominal() {
    eng::engine l_engine;
    l_engine.allocate();

    eng::window_handle l_window = l_engine.window_open(100, 100);
    l_engine.update();
    l_engine.window_close(l_window);

    l_engine.free();
  };
};
}; // namespace

inline static void test_window() { window_tests::nominal(); };