#pragma once

#include <sys/sys.hpp>

#if DEBUG_PREPROCESS
#define assert_debug(p_condition) sys::sassert(p_condition)
#define block_debug(p_code) p_code
#else
#define assert_debug(p_condition)
#define block_debug(p_code)
#endif