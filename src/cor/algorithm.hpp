#pragma once

#include <cor/cor.hpp>

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

}; // namespace algorithm