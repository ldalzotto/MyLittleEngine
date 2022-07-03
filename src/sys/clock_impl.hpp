#pragma once

#if PLATFORM_WEBASSEMBLY_PREPROCESS
#include <sys/clock_emscripten_impl.hpp>
#else
#include <sys/clock_linux_impl.hpp>
#endif