#pragma once

#if WIN_HEADLESS_PREPROCESS
#include <sys/win_headless_impl.hpp>
#else

#if PLATFORM_WEBASSEMBLY_PREPROCESS
#include <sys/win_emscripten_impl.hpp>
#else
#include <sys/win_linux_impl.hpp>
#endif


#endif
