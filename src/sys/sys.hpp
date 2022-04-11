#pragma once

#include <cor/cor.hpp>

namespace {
struct sys {
  static void *malloc(ui64 p_size);
  static void free(void *p_ptr);
  static void *realloc(void *p_ptr, ui64 p_new_size);
};
} // namespace
