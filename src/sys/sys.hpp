#pragma once

#include <cor/cor.hpp>

struct sys {
  static void *malloc(uimax_t p_size);
  static void free(void *p_ptr);
  static void *realloc(void *p_ptr, uimax_t p_new_size);
  static void memmove(void *p_dest, void *p_src, uimax_t p_n);

  template <typename T>
  inline static void memmove_up(T *p_ptr, uimax_t p_break_index,
                                uimax_t p_move_delta, uimax_t p_chunk_count) {
    T *l_src = p_ptr + p_break_index;
    T *l_dst = l_src - p_move_delta;
    uimax_t l_byte_size = p_chunk_count * sizeof(T);
    sys::memmove(l_dst, l_src, l_byte_size);
  };

  static void memcpy(void *p_dest, void *p_src, uimax_t p_n);
  static void sassert(bool p_condition);
  static void abort();
};