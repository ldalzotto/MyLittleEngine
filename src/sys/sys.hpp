#pragma once

#include <cor/cor.hpp>

namespace {
struct sys {
  static void *malloc(uimax p_size);
  static void free(void *p_ptr);
  static void *realloc(void *p_ptr, uimax p_new_size);
  static void sassert(bool p_condition);
};
} // namespace
