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

  inline static f32 stof(const ui8 *p_value, ui8 p_count) {
    f32 l_return = 0;
    ui8 l_separation_index = p_count - 1;
    for (auto i = 0; i < p_count; ++i) {
      if (p_value[i] == '.') {
        l_separation_index = i;
        break;
      }
    }

    ui8 l_negate = 0;
    if (p_value[0] == '-') {
      l_negate = 1;
    }

    auto l_right_begin = l_negate;
    auto l_right_end = l_separation_index;
    auto l_right_count = l_right_end - l_right_begin;

    auto l_left_begin = l_separation_index + 1;
    auto l_left_end = p_count;
    auto l_left_count = l_left_end - l_left_begin;

    f32 l_pow = 1.0f;
    for (auto i = l_right_end; i > l_right_begin; --i) {
      const f32 l_value = f32(p_value[i - 1] - 48);
      l_return += l_value * l_pow;
      l_pow *= 10;
    }

    l_pow = 0.1f;
    for (auto i = l_left_begin; i < l_left_end; ++i) {
      const f32 l_value = f32(p_value[i] - 48);
      l_return += l_value * l_pow;
      l_pow /= 10;
    }

    if (l_negate) {
      l_return *= -1;
    }

    return l_return;
  };

  template <typename T> inline static T stoui(const ui8 *p_value, ui8 p_count) {
    T l_return = 0;
    T l_pow = 1;
    for (auto i = p_count - 1; i >= 0; --i) {
      const T l_value = T(p_value[i]) - 48;
      l_return += (l_value * l_pow);
      l_pow *= 10;
    }
    return l_return;
  };

  template <typename T> inline static uimax strlen(const T *p_value) {
    uimax l_it = 0;
    while (p_value[l_it] != '\0') {
      l_it += 1;
    }
    return l_it;
  };
};

#define FORCE_INLINE __attribute__((always_inline)) inline