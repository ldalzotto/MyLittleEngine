#pragma once

#include <cor/types.hpp>
#include <sys/sys.hpp>

struct malloc_free_functions {
  static void *malloc(uimax p_size) { return sys::malloc(p_size); };
  static void free(void *p_ptr) { sys::free(p_ptr); };
  static void *realloc(void *p_ptr, uimax p_size) {
    return sys::realloc(p_ptr, p_size);
  };
};
