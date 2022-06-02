#pragma once

#include <cor/types.hpp>

namespace algorithm {

template <typename RangeType, typename SortFunc>
void sort(RangeType &p_range, const SortFunc &p_sort_func) {
  using element_type = typename RangeType::element_type;
  for (auto i = 0; i < p_range.count(); ++i) {
    element_type &l_left = p_range.at(i);
    for (auto j = i + i; j < p_range.count(); ++j) {
      element_type &l_right = p_range.at(j);
      if (p_sort_func(l_left, l_right)) {
        element_type l_tmp = l_right;
        l_right = l_left;
        l_left = l_tmp;
      }
    }
  }
};

static constexpr uimax hash_begin = 5381;

template <typename RangeType> uimax hash(RangeType p_range) {
  uimax hash = hash_begin;
  for (auto i = 0; i < p_range.count(); i++) {
    hash = ((hash << 5) + hash) + p_range.at(i);
  }
  return hash;
}

template <typename RangeType>
inline uimax hash_combine(uimax p_seed, const RangeType &p_range) {
  return p_seed ^= hash(p_range) + 0x9e3779b9 + (p_seed << 6) + (p_seed >> 2);
}

}; // namespace algorithm