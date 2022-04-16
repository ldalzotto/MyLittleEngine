#pragma once

#include <cor/cor.hpp>

namespace hash {

uimax_t hash_function(ui8_t *p_str, uimax_t p_count) {
  uimax_t l_hash = 5381;

  uimax_t l_index = 0;
  for (auto l_index = 0; l_index < p_count; ++l_index) {
    l_hash = ((l_hash << 5) + l_hash) + p_str[l_index]; /* hash * 33 + c */
  }

  return l_hash;
};

template <typename T> struct hash {
  hash() = delete;
  uimax_t operator()(const T &p_value) {
    return hash_function(&p_value, sizeof(T));
  };
};

}; // namespace hash
