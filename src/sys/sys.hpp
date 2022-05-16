#pragma once

#include <cor/types.hpp>

struct sys {
  static void *malloc(uimax p_size);
  static void free(void *p_ptr);
  static void *realloc(void *p_ptr, uimax p_new_size);
  static void memmove(void *p_dest, void *p_src, uimax p_n);
  static void memcpy(void *p_dest, void *p_src, uimax p_n);
  static void memset(void *p_dest, ui32 p_value, uimax p_n);
  static ui8 memcmp(void *p_left, void *p_right, uimax p_n);

  template <typename T>
  inline static void memmove_up_t(T *p_ptr, uimax p_break_index,
                                  uimax p_move_delta, uimax p_chunk_count) {
    T *l_src = p_ptr + p_break_index;
    T *l_dst = l_src - p_move_delta;
    uimax l_byte_size = p_chunk_count * sizeof(T);
    sys::memmove(l_dst, l_src, l_byte_size);
  };

  template <typename T>
  inline static void memmove_down_t(T *p_ptr, uimax p_break_index,
                                    uimax p_move_delta, uimax p_chunk_count) {
    T *l_src = p_ptr + p_break_index;
    T *l_dst = l_src + p_move_delta;
    uimax l_byte_size = p_chunk_count * sizeof(T);
    sys::memmove(l_dst, l_src, l_byte_size);
  };

  static void sassert(bool p_condition);
  static void abort();

  static f32 sin(f32 p_angle);
  static f32 cos(f32 p_angle);
  static f32 tan(f32 p_angle);

  static inline i32 nearest(f32 v) {
    i32 lx = (i32)v;
    i32 lxr = (i32)(v + 0.5f);
    if (lxr == lx) {
      return lx;
    }
    return lxr;
  };
};

#define FORCE_INLINE __attribute__((always_inline)) inline