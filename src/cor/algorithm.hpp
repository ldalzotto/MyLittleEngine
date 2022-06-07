#pragma once

#include <cor/traits.hpp>
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
};

template <typename RangeType>
inline uimax hash_combine(uimax p_seed, const RangeType &p_range) {
  return p_seed ^= hash(p_range) + 0x9e3779b9 + (p_seed << 6) + (p_seed >> 2);
};

template <typename StrRangeType> struct str_iterator {
  StrRangeType &m_str;
  uimax m_char_iterator;

  str_iterator(StrRangeType &p_str) : m_str(p_str) { m_char_iterator = 0; };

  void advance(uimax p_offset) { m_char_iterator += p_offset; };

  StrRangeType range_next_char(i8 p_char) {
    uimax l_start_it = m_char_iterator;
    while (m_str.at(m_char_iterator) != p_char) {
      m_char_iterator += 1;
    }
    return m_str.slide(l_start_it).shrink_to(m_char_iterator - l_start_it);
  };

  StrRangeType range_next_char_advance(i8 p_char) {
    StrRangeType l_return = range_next_char(p_char);
    m_char_iterator += 1;
    return l_return;
  };

  StrRangeType range_until_end() {
    uimax l_start_it = m_char_iterator;
    while (m_char_iterator != m_str.count()) {
      m_char_iterator += 1;
    }
    return m_str.slide(l_start_it).shrink_to(m_char_iterator - l_start_it);
  };

  template <typename CallbackFunc>
  void split(i8 p_char, const CallbackFunc &p_cb) {
    uimax l_it = 0;
    uimax l_begin = l_it;
    uimax l_end = l_begin;
    while (true) {
      if (l_it == m_str.count() || m_str.at(l_it) == p_char) {
        l_end = l_it;
        auto l_range = m_str.slide(l_begin).shrink_to(l_end - l_begin);
        p_cb(l_range);
        l_begin = l_it + 1;
        l_end = l_begin;
      }

      if (l_it == m_str.count()) {
        break;
      }

      l_it += 1;
    }
  };
};

template <typename StrRangeType> struct str_line_iterator {
  using line_t = typename traits::remove_const<StrRangeType>::type;
  using element_type_t = typename line_t::element_type;

  StrRangeType &m_str;
  line_t m_line;
  uimax m_absolute_iterator;

  uimax &index() { return m_absolute_iterator; };
  line_t &line() { return m_line; };

  str_line_iterator(StrRangeType &p_str) : m_str(p_str) {
    m_line = StrRangeType::make(0, 0);
    m_absolute_iterator = 0;
  };

  template <typename CallbackFunc>
  void for_each_line(const CallbackFunc &p_cb) {
    m_absolute_iterator = 0;
    while (__next_line()) {
      p_cb(m_line);
    }
  };

  void set_iterator_index(uimax p_iterator) {
    m_absolute_iterator = p_iterator;
  };
  line_t &next_line() {
    __next_line();
    return m_line;
  };

  uimax line_begin_absolute() const {
    return m_absolute_iterator - m_line.count() - 1;
  };

private:
  ui8 __next_line() {
    m_line = line_t::make((element_type_t *)&m_str.at(m_absolute_iterator), 0);
    while (true) {

      if (m_absolute_iterator == m_str.count() - 1) {
        return 0;
      }

      if (m_str.at(m_absolute_iterator) == '\n') {
        if ((m_absolute_iterator + 1) == m_str.count() - 1) {
          return 0;
        } else {
          m_absolute_iterator += 1;
          return 1;
        }
      }

      m_absolute_iterator += 1;
      m_line.count() += 1;
    }
  };
};

}; // namespace algorithm

namespace container {
namespace algorithm {

template <typename SpanType, typename AllocationFunc>
void span_allocate(SpanType &thiz, uimax p_count,
                   const AllocationFunc &p_allocation_func) {
  p_allocation_func();
  thiz.count() = p_count;
};


template <typename SpanType, typename ReallocFunc>
void span_realloc(SpanType &thiz, uimax p_new_count,
                  const ReallocFunc &p_realloc_func) {
  thiz.count() = p_new_count;
  p_realloc_func();
};

template <typename SpanType, typename ReallocFunc>
void span_resize(SpanType &thiz, uimax p_new_count,
                 const ReallocFunc &p_realloc_func) {
  if (p_new_count > thiz.count()) {
    span_realloc(thiz, p_new_count, p_realloc_func);
  }
};

}; // namespace algorithm
}; // namespace container