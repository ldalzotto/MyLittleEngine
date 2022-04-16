#pragma once

#include <cor/cor.hpp>

struct sys {
  static void *malloc(uimax_t p_size);
  static void free(void *p_ptr);
  static void *realloc(void *p_ptr, uimax_t p_new_size);
  static void memmove(void* p_dest, void* p_src, uimax_t p_n);
  static void memcpy(void* p_dest, void* p_src, uimax_t p_n);
  static void sassert(bool p_condition);
  static void abort();
};