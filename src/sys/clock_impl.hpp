#pragma once

#if CLOCK_FIXED_PREPROCESS
#include <sys/clock_fixed_impl.hpp>
#else
#include <sys/clock_linux_impl.hpp>
#endif
