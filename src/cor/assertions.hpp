#pragma once

#include <sys/sys.hpp>

template <typename CallbacType> void block_debug(const CallbacType &p_cb) {
#if DEBUG_PREPROCESS
  p_cb();
#else
#endif
};

#if DEBUG_PREPROCESS
#define assert_debug(p_condition) sys::sassert(p_condition)
#else
#define assert_debug(p_condition)
#endif