#pragma once

#include <cor/container.hpp>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace orm {

struct none {};

namespace traits {
template <typename T> struct is_none { static constexpr ui8 value = 0; };

template <> struct is_none<orm::none> { static constexpr ui8 value = 1; };
}; // namespace traits

namespace details {

template <typename TableType>
void __allocate_pool_1(TableType &p_table, uimax p_capacity) {
  p_table.m_meta.allocate(p_capacity);
  p_table.m_col_0 = (typename TableType::type_0 *)sys::malloc(
      sizeof(typename TableType::type_0) * p_capacity);
};

template <typename TableType> void __free_pool_1(TableType &p_table) {
  p_table.m_meta.free();
  sys::free(p_table.m_col_0);
};

template <typename TableType> void __realloc_pool_1(TableType &p_table) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  p_table.m_col_0 = (decltype(p_table.m_col_0))sys::realloc(
      p_table.m_col_0, l_meta.m_capacity * sizeof(*p_table.m_col_0));
};

template <typename TableType>
uimax __push_back_pool_1(TableType &p_table,
                         const typename TableType::type_0 &p_0) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  uimax l_index;
  if (l_meta.find_next_realloc(&l_index)) {
    __realloc_pool_1(p_table);
  }
  p_table.m_col_0[l_index] = p_0;
  return l_index;
};

template <typename TableType>
void __at_pool_1(TableType &p_table, uimax p_index,
                 typename TableType::type_0 **out_0) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  assert_debug(p_index < l_meta.m_count);
  *out_0 = &p_table.m_col_0[p_index];
};

template <typename TableType>
void __allocate_pool_2(TableType &p_table, uimax p_capacity) {
  p_table.m_meta.allocate(p_capacity);
  p_table.m_col_0 = (typename TableType::type_0 *)sys::malloc(
      sizeof(typename TableType::type_0) * p_capacity);
  p_table.m_col_1 = (typename TableType::type_1 *)sys::malloc(
      sizeof(typename TableType::type_1) * p_capacity);
};

template <typename TableType> void __free_pool_2(TableType &p_table) {
  p_table.m_meta.free();
  sys::free(p_table.m_col_0);
  sys::free(p_table.m_col_1);
};

template <typename TableType> void __realloc_pool_2(TableType &p_table) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  p_table.m_col_0 = (decltype(p_table.m_col_0))sys::realloc(
      p_table.m_col_0, l_meta.m_capacity * sizeof(*p_table.m_col_0));
  p_table.m_col_1 = (decltype(p_table.m_col_1))sys::realloc(
      p_table.m_col_1, l_meta.m_capacity * sizeof(*p_table.m_col_1));
};

template <typename TableType>
uimax __push_back_pool_2(TableType &p_table,
                         const typename TableType::type_0 &p_0,
                         const typename TableType::type_1 &p_1) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  uimax l_index;
  if (l_meta.find_next_realloc(&l_index)) {
    __realloc_pool_2(p_table);
  }
  p_table.m_col_0[l_index] = p_0;
  p_table.m_col_1[l_index] = p_1;
  return l_index;
};

template <typename TableType>
uimax __push_back_pool_2(TableType &p_table, const orm::none &,
                         const typename TableType::type_1 &p_1) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  uimax l_index;
  if (l_meta.find_next_realloc(&l_index)) {
    __realloc_pool_2(p_table);
  }
  p_table.m_col_1[l_index] = p_1;
  return l_index;
};

template <typename TableType>
uimax __push_back_pool_2(TableType &p_table,
                         const typename TableType::type_0 &p_0,
                         const orm::none &) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  uimax l_index;
  if (l_meta.find_next_realloc(&l_index)) {
    __realloc_pool_2(p_table);
  }
  p_table.m_col_0[l_index] = p_0;
  return l_index;
};

template <typename TableType>
void __at_pool_2(TableType &p_table, uimax p_index,
                 typename TableType::type_0 **out_0,
                 typename TableType::type_1 **out_1) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  assert_debug(p_index < l_meta.m_count);
  *out_0 = &p_table.m_col_0[p_index];
  *out_1 = &p_table.m_col_1[p_index];
};

template <typename TableType>
void __at_pool_2(TableType &p_table, uimax p_index, orm::none,
                 typename TableType::type_1 **out_1) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  assert_debug(p_index < l_meta.m_count);
  *out_1 = &p_table.m_col_1[p_index];
};

template <typename TableType>
void __at_pool_2(TableType &p_table, uimax p_index,
                 typename TableType::type_0 **out_0, orm::none) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  assert_debug(p_index < l_meta.m_count);
  *out_0 = &p_table.m_col_0[p_index];
};

template <typename TableType>
void __remove_at_pool(TableType &p_table, uimax p_index) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  l_meta.free_element(p_index);
};

template <typename TableType>
bool __has_allocated_elements_pool(TableType &p_table) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  return l_meta.has_allocated_elements();
};

template <typename T> struct heap_paged_col {
  T **m_data;

  void allocate() { m_data = (T **)sys::malloc(0); };

  void free(const container::heap_paged_intrusive &p_intrusive) {
    for (auto i = 0; i < p_intrusive.m_pages_intrusive.m_count; ++i) {
      sys::free(m_data[i]);
    }
    sys::free(m_data);
  };

  void realloc(const container::heap_paged_intrusive &p_intrusive) {
    m_data = (T **)sys::realloc(
        m_data, p_intrusive.m_pages_intrusive.m_capacity * sizeof(*m_data));
  };

  void allocate_page(const container::heap_paged_intrusive &p_intrusive,
                     uimax p_page_index) {
    m_data[p_page_index] =
        (T *)sys::malloc(sizeof(T) * p_intrusive.m_single_page_capacity);
  };

  container::range<T> map_to_range(const container::heap_paged_chunk &p_chunk) {
    container::range<T> l_range;
    l_range.m_begin = &(m_data[p_chunk.m_page_index])[p_chunk.m_chunk.m_begin];
    l_range.m_count = p_chunk.m_chunk.m_size;
    return l_range;
  };
};

template <typename TableType>
void __allocate_heap_paged_1(TableType &p_table, uimax p_capacity) {
  using type_0 = typename TableType::type_0;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  l_meta.allocate(p_capacity);
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  l_col_0.allocate();
};

template <typename TableType> void __free_heap_paged_1(TableType &p_table) {
  using type_0 = typename TableType::type_0;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  l_meta.free();
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  l_col_0.free(l_meta);
};

template <typename TableType>
uimax __at_heap_paged_1(TableType &p_table, uimax p_chunk_index,
                        typename TableType::type_0 **out_0) {
  using type_0 = typename TableType::type_0;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  container::range<type_0> l_range_0 =
      l_col_0.map_to_range(l_meta.m_allocated_chunks.at(p_chunk_index));
  *out_0 = l_range_0.m_begin;
  return l_range_0.m_count;
};

template <typename TableType>
uimax __push_back_heap_paged_1(TableType &p_table, uimax p_size) {
  using type_0 = typename TableType::type_0;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;

  uimax l_page_index, l_chunk_index;
  l_meta.find_next_chunk(p_size, &l_page_index, &l_chunk_index);
  if (l_meta.m_state == container::heap_paged_intrusive::state::NewPagePushed) {
    l_meta.clear_state();
    l_col_0.realloc(l_meta);
    l_col_0.allocate_page(l_meta, l_page_index);
  }

  return l_meta.push_found_chunk(p_size, l_page_index, l_chunk_index);
};

template <typename TableType>
void __remove_at_heap_paged_1(TableType &p_table, uimax p_chunk_index) {
  using type_0 = typename TableType::type_0;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  l_meta.remove_chunk(p_chunk_index);
};

template <typename TableType>
bool __has_allocated_elements_heap_paged(TableType &p_table) {
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  return l_meta.m_allocated_chunks.m_intrusive.has_allocated_elements();
};

template <typename TableType>
void __allocate_heap_paged_2(TableType &p_table, uimax p_capacity) {
  using type_0 = typename TableType::type_0;
  using type_1 = typename TableType::type_1;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  l_meta.allocate(p_capacity);
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  heap_paged_col<type_1> &l_col_1 = p_table.m_col_1;
  l_col_0.allocate();
  l_col_1.allocate();
};

template <typename TableType> void __free_heap_paged_2(TableType &p_table) {
  using type_0 = typename TableType::type_0;
  using type_1 = typename TableType::type_1;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  l_meta.free();
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  heap_paged_col<type_1> &l_col_1 = p_table.m_col_1;
  l_col_0.free(l_meta);
  l_col_1.free(l_meta);
};

template <typename TableType>
uimax __at_heap_paged_2(TableType &p_table, uimax p_chunk_index,
                        typename TableType::type_0 **out_0,
                        typename TableType::type_1 **out_1) {
  using type_0 = typename TableType::type_0;
  using type_1 = typename TableType::type_1;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  heap_paged_col<type_1> &l_col_1 = p_table.m_col_1;
  container::range<type_0> l_range_0 =
      l_col_0.map_to_range(l_meta.m_allocated_chunks.at(p_chunk_index));
  container::range<type_1> l_range_1 =
      l_col_1.map_to_range(l_meta.m_allocated_chunks.at(p_chunk_index));
  assert_debug(l_range_0.m_count == l_range_1.m_count);
  *out_0 = l_range_0.m_begin;
  *out_1 = l_range_1.m_begin;
  return l_range_0.m_count;
};

template <typename TableType>
uimax __at_heap_paged_2(TableType &p_table, uimax p_chunk_index, orm::none,
                        typename TableType::type_1 **out_1) {
  using type_1 = typename TableType::type_1;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  heap_paged_col<type_1> &l_col_1 = p_table.m_col_1;
  container::range<type_1> l_range_1 =
      l_col_1.map_to_range(l_meta.m_allocated_chunks.at(p_chunk_index));
  *out_1 = l_range_1.m_begin;
  return l_range_1.m_count;
};

template <typename TableType>
uimax __push_back_heap_paged_2(TableType &p_table, uimax p_size) {
  using type_0 = typename TableType::type_0;
  using type_1 = typename TableType::type_1;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  heap_paged_col<type_1> &l_col_1 = p_table.m_col_1;

  uimax l_page_index, l_chunk_index;
  l_meta.find_next_chunk(p_size, &l_page_index, &l_chunk_index);
  if (l_meta.m_state == container::heap_paged_intrusive::state::NewPagePushed) {
    l_meta.clear_state();
    l_col_0.realloc(l_meta);
    l_col_0.allocate_page(l_meta, l_page_index);
    l_col_1.realloc(l_meta);
    l_col_1.allocate_page(l_meta, l_page_index);
  }

  return l_meta.push_found_chunk(p_size, l_page_index, l_chunk_index);
};

template <typename TableType>
void __remove_at_heap_paged_2(TableType &p_table, uimax p_chunk_index) {
  using type_0 = typename TableType::type_0;
  using type_1 = typename TableType::type_1;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  l_meta.remove_chunk(p_chunk_index);
};

template <typename TableType>
void __allocate_heap_paged_3(TableType &p_table, uimax p_capacity) {
  using type_0 = typename TableType::type_0;
  using type_1 = typename TableType::type_1;
  using type_2 = typename TableType::type_2;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  l_meta.allocate(p_capacity);
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  heap_paged_col<type_1> &l_col_1 = p_table.m_col_1;
  heap_paged_col<type_1> &l_col_2 = p_table.m_col_2;
  l_col_0.allocate();
  l_col_1.allocate();
  l_col_2.allocate();
};

template <typename TableType> void __free_heap_paged_3(TableType &p_table) {
  using type_0 = typename TableType::type_0;
  using type_1 = typename TableType::type_1;
  using type_2 = typename TableType::type_2;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  l_meta.free();
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  heap_paged_col<type_1> &l_col_1 = p_table.m_col_1;
  heap_paged_col<type_2> &l_col_2 = p_table.m_col_2;
  l_col_0.free();
  l_col_1.free();
  l_col_2.free();
};

template <typename TableType>
void __at_heap_paged_3(TableType &p_table, uimax p_chunk_index,
                       typename TableType::type_0 **out_0,
                       typename TableType::type_1 **out_1,
                       typename TableType::type_2 **out_2, uimax *out_count) {
  using type_0 = typename TableType::type_0;
  using type_1 = typename TableType::type_1;
  using type_2 = typename TableType::type_2;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  heap_paged_col<type_1> &l_col_1 = p_table.m_col_1;
  heap_paged_col<type_2> &l_col_2 = p_table.m_col_2;
  container::range<type_0> l_range_0 =
      l_col_0.map_to_range(l_meta.m_allocated_chunks.at(p_chunk_index));
  container::range<type_1> l_range_1 =
      l_col_1.map_to_range(l_meta.m_allocated_chunks.at(p_chunk_index));
  container::range<type_2> l_range_2 =
      l_col_2.map_to_range(l_meta.m_allocated_chunks.at(p_chunk_index));
  assert_debug(l_range_0.m_count == l_range_1.m_count);
  assert_debug(l_range_0.m_count == l_range_2.m_count);
  *out_0 = l_range_0;
  *out_1 = l_range_1;
  *out_2 = l_range_2;
  *out_count = l_range_0.m_count;
};

template <typename TableType>
uimax __push_back_heap_paged_3(TableType &p_table, uimax p_size) {
  using type_0 = typename TableType::type_0;
  using type_1 = typename TableType::type_1;
  using type_2 = typename TableType::type_2;
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  heap_paged_col<type_0> &l_col_0 = p_table.m_col_0;
  heap_paged_col<type_1> &l_col_1 = p_table.m_col_1;
  heap_paged_col<type_2> &l_col_2 = p_table.m_col_2;

  uimax l_page_index, l_chunk_index;
  l_meta.find_next_chunk(p_size, &l_page_index, &l_chunk_index);
  if (l_meta.m_state == container::heap_paged_intrusive::state::NewPagePushed) {
    l_meta.clear_state();
    l_col_0.realloc(l_meta);
    l_col_0.allocate_page(l_meta, l_page_index);
    l_col_1.realloc(l_meta);
    l_col_1.allocate_page(l_meta, l_page_index);
    l_col_2.realloc(l_meta);
    l_col_2.allocate_page(l_meta, l_page_index);
  }

  return l_meta.push_found_chunk(p_size, l_page_index, l_chunk_index);
};

template <typename TableType>
void __remove_at_heap_paged_3(TableType &p_table, uimax p_chunk_index) {
  container::heap_paged_intrusive &l_meta = p_table.m_meta;
  l_meta.remove_chunk(p_chunk_index);
};

}; // namespace details

struct table_one_to_many {

  struct entry {
    uimax m_left_row_id;
    container::vector<uimax> m_links;

    void allocate(uimax p_capacity) { m_links.allocate(p_capacity); };
    void free() { m_links.free(); };

    i8 is_linked_to(uimax p_link) {
      for (auto i = 0; i < m_links.count(); ++i) {
        if (m_links.at(i) == p_link) {
          return 1;
        }
      }
      return 0;
    };

    void push_link(uimax p_other) {
      assert_debug(!is_linked_to(p_other));
      m_links.push_back(p_other);
    };

    void remove_link(uimax p_other) {
      assert_debug(is_linked_to(p_other));
      for (auto i = 0; i < m_links.count(); ++i) {
        if (m_links.at(i) == p_other) {
          m_links.remove_at(i);
          break;
        }
      }
      assert_debug(!is_linked_to(p_other));
    };
  };

  container::vector<entry> m_entries;

  void allocate(uimax p_capacity) { m_entries.allocate(p_capacity); };

  void free() {
    for (auto i = 0; i < m_entries.count(); ++i) {
      m_entries.at(i).free();
    }
    m_entries.free();
  };

  void push_back(uimax p_left, uimax p_right) {
    entry *l_entry = __get_entry(p_left);
    if (!l_entry) {
      entry tmp_entry;
      tmp_entry.allocate(0);
      tmp_entry.m_left_row_id = p_left;
      m_entries.push_back(tmp_entry);
      l_entry = &m_entries.at(m_entries.count() - 1);
    }

    assert_debug(l_entry);
    l_entry->push_link(p_right);
  };

  void remove(uimax p_left, uimax p_right) {
    entry *l_entry = __get_entry(p_left);
    assert_debug(l_entry);
    l_entry->remove_link(p_right);
  };

private:
  entry *__get_entry(uimax p_left_row_id) {
    for (auto i = 0; i < m_entries.count(); ++i) {
      if (m_entries.at(i).m_left_row_id == p_left_row_id) {
        return &m_entries.at(i);
      }
    }
    return 0;
  };
};

#define table_cols_1(Type0)                                                    \
  using type_0 = Type0;                                                        \
  type_0 *m_col_0;

#define table_cols_2(Type0, Type1)                                             \
  using type_0 = Type0;                                                        \
  using type_1 = Type1;                                                        \
  type_0 *m_col_0;                                                             \
  type_1 *m_col_1;

#define table_cols_3(Type0, Type1, Type2)                                      \
  using type_0 = Type0;                                                        \
  using type_1 = Type1;                                                        \
  using type_2 = Type2;                                                        \
  type_0 *m_col_0;                                                             \
  type_1 *m_col_1;                                                             \
  type_2 *m_col_2;

#define table_cols_4(Type0, Type1, Type2, Type3)                               \
  using type_0 = Type0;                                                        \
  using type_1 = Type1;                                                        \
  using type_2 = Type2;                                                        \
  using type_3 = Type3;                                                        \
  type_0 *m_col_0;                                                             \
  type_1 *m_col_1;                                                             \
  type_2 *m_col_2;                                                             \
  type_3 *m_col_3;

#define table_pool_meta container::pool_intrusive m_meta;

#define table_define_pool_1                                                    \
  void allocate(uimax p_capacity) {                                            \
    orm::details::__allocate_pool_1(*this, p_capacity);                        \
  };                                                                           \
  void free() { orm::details::__free_pool_1(*this); };                         \
  uimax push_back(const type_0 &p_0) {                                         \
    return orm::details::__push_back_pool_1(*this, p_0);                       \
  };                                                                           \
  void remove_at(uimax p_index) {                                              \
    orm::details::__remove_at_pool(*this, p_index);                            \
  };                                                                           \
  bool has_allocated_elements() {                                              \
    return orm::details::__has_allocated_elements_pool(*this);                 \
  };                                                                           \
  void at(uimax p_index, type_0 **out_0) {                                     \
    orm::details::__at_pool_1(*this, p_index, out_0);                          \
  };

#define table_define_pool_2                                                    \
  void allocate(uimax p_capacity) {                                            \
    orm::details::__allocate_pool_2(*this, p_capacity);                        \
  };                                                                           \
  void free() { orm::details::__free_pool_2(*this); };                         \
  uimax push_back(const type_0 &p_0, const type_1 &p_1) {                      \
    return orm::details::__push_back_pool_2(*this, p_0, p_1);                  \
  };                                                                           \
  uimax push_back(const type_0 &p_0, const orm::none &p_none) {                \
    return orm::details::__push_back_pool_2(*this, p_0, p_none);               \
  };                                                                           \
  uimax push_back(const orm::none &p_none, const type_1 &p_1) {                \
    return orm::details::__push_back_pool_2(*this, p_none, p_1);               \
  };                                                                           \
  void remove_at(uimax p_index) {                                              \
    orm::details::__remove_at_pool(*this, p_index);                            \
  };                                                                           \
  bool has_allocated_elements() {                                              \
    return orm::details::__has_allocated_elements_pool(*this);                 \
  };                                                                           \
  void at(uimax p_index, type_0 **out_0, type_1 **out_1) {                     \
    orm::details::__at_pool_2(*this, p_index, out_0, out_1);                   \
  };                                                                           \
  void at(uimax p_index, const orm::none &p_none, type_1 **out_1) {            \
    orm::details::__at_pool_2(*this, p_index, p_none, out_1);                  \
  };                                                                           \
  void at(uimax p_index, type_0 **out_0, const orm::none &p_none) {            \
    orm::details::__at_pool_2(*this, p_index, out_0, p_none);                  \
  };

#define table_heap_paged_meta container::heap_paged_intrusive m_meta;
#define table_heap_paged_cols_1(Type0)                                         \
  using type_0 = Type0;                                                        \
  orm::details::heap_paged_col<type_0> m_col_0;

#define table_define_heap_paged_1                                              \
  void allocate(uimax p_capacity) {                                            \
    orm::details::__allocate_heap_paged_1(*this, p_capacity);                  \
  };                                                                           \
  void free() { orm::details::__free_heap_paged_1(*this); };                   \
  uimax push_back(uimax p_count) {                                             \
    return orm::details::__push_back_heap_paged_1(*this, p_count);             \
  };                                                                           \
  uimax at(uimax p_index, type_0 **out_0) {                                    \
    return orm::details::__at_heap_paged_1(*this, p_index, out_0);             \
  };                                                                           \
  void remove_at(uimax p_index) {                                              \
    orm::details::__remove_at_heap_paged_1(*this, p_index);                    \
  };                                                                           \
  bool has_allocated_elements() {                                              \
    return orm::details::__has_allocated_elements_heap_paged(*this);           \
  };

#define table_heap_paged_cols_2(Type0, Type1)                                  \
  using type_0 = Type0;                                                        \
  using type_1 = Type1;                                                        \
  orm::details::heap_paged_col<type_0> m_col_0;                                \
  orm::details::heap_paged_col<type_1> m_col_1;

#define table_define_heap_paged_2                                              \
  void allocate(uimax p_capacity) {                                            \
    orm::details::__allocate_heap_paged_2(*this, p_capacity);                  \
  };                                                                           \
  void free() { orm::details::__free_heap_paged_2(*this); };                   \
  uimax push_back(uimax p_count) {                                             \
    return orm::details::__push_back_heap_paged_2(*this, p_count);             \
  };                                                                           \
  uimax at(uimax p_index, type_0 **out_0, type_1 **out_1) {                    \
    return orm::details::__at_heap_paged_2(*this, p_index, out_0, out_1);      \
  };                                                                           \
  uimax at(uimax p_index, orm::none p_none, type_1 **out_1) {                  \
    return orm::details::__at_heap_paged_2(*this, p_index, p_none, out_1);     \
  };                                                                           \
  void remove_at(uimax p_index) {                                              \
    orm::details::__remove_at_heap_paged_2(*this, p_index);                    \
  };                                                                           \
  bool has_allocated_elements() {                                              \
    return orm::details::__has_allocated_elements_heap_paged(*this);           \
  };

#define table_heap_paged_cols_3(Type0, Type1, Type2)                           \
  using type_0 = Type0;                                                        \
  using type_1 = Type1;                                                        \
  using type_2 = Type2;                                                        \
  orm::details::heap_paged_col<type_0> m_col_0;                                \
  orm::details::heap_paged_col<type_1> m_col_1;                                \
  orm::details::heap_paged_col<type_2> m_col_2;

#define table_define_heap_paged_3                                              \
  void allocate(uimax p_capacity) {                                            \
    orm::details::__allocate_heap_paged_3(*this, p_capacity);                  \
  };                                                                           \
  void free() { orm::details::__free_heap_paged_3(*this); };                   \
  uimax push_back(uimax p_count) {                                             \
    return orm::details::__push_back_heap_paged_3(*this, p_count);             \
  };                                                                           \
  void at(uimax p_index, type_0 **out_0, type_1 **out_1, type_2 **out_2,       \
          uimax *out_count) {                                                  \
    orm::details::__at_heap_paged_3(*this, p_index, out_0, out_1, out_2,       \
                                    out_count);                                \
  };                                                                           \
  void remove_at(uimax p_index) {                                              \
    orm::details::__remove_at_heap_paged_3(*this, p_index);                    \
  };                                                                           \
  bool has_allocated_elements() {                                              \
    return orm::details::__has_allocated_elements_heap_paged(*this);           \
  };

}; // namespace orm

namespace orm {

template <typename T, ui8 Col> struct ref {
  using type = T;
  static constexpr ui8 COL = Col;

  operator T &() { return *m_data; };
  operator T * &() { return m_data; };
  T *operator->() { return m_data; };
  T &operator*() { return *m_data; }

private:
  T *m_data;

public:
  T *&data() { return m_data; };
};

namespace traits {
template <typename T> struct is_orm_ref { static constexpr ui8 value = 0; };

template <typename T, ui8 Col> struct is_orm_ref<ref<T, Col>> {
  static constexpr ui8 value = 1;
};
}; // namespace traits

namespace details {

template <typename... Types> struct cols;
template <typename Type0> struct cols<Type0> {
  static constexpr ui8 COL_COUNT = 1;
  Type0 *m_col_0;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
};

template <typename Type0, typename Type1> struct cols<Type0, Type1> {
  static constexpr ui8 COL_COUNT = 2;
  Type0 *m_col_0;
  Type1 *m_col_1;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
  template <> auto &col<1>() { return m_col_1; };
};

template <typename Type0, typename Type1, typename Type2>
struct cols<Type0, Type1, Type2> {
  static constexpr ui8 COL_COUNT = 3;
  Type0 *m_col_0;
  Type1 *m_col_1;
  Type2 *m_col_2;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
  template <> auto &col<1>() { return m_col_1; };
  template <> auto &col<2>() { return m_col_2; };
};

template <typename Type0, typename Type1, typename Type2, typename Type3>
struct cols<Type0, Type1, Type2, Type3> {
  static constexpr ui8 COL_COUNT = 4;
  Type0 *m_col_0;
  Type1 *m_col_1;
  Type2 *m_col_2;
  Type3 *m_col_3;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
  template <> auto &col<1>() { return m_col_1; };
  template <> auto &col<2>() { return m_col_2; };
  template <> auto &col<3>() { return m_col_3; };
};

}; // namespace details

template <typename... Types> struct table_span_v2 {
  uimax m_meta;
  details::cols<Types...> m_cols;
  auto &cols() { return m_cols; };

  inline static constexpr ui8 COL_COUNT = details::cols<Types...>::COL_COUNT;

  void allocate(uimax p_count) { table_span_allocate{}(*this, p_count); };

  void free() { table_span_free{}(*this); };

  void realloc(uimax p_new_count) { table_span_realloc{}(*this, p_new_count); };

  void resize(uimax p_new_count) {
    if (p_new_count > count()) {
      realloc(p_new_count);
    }
  };

  uimax &count() { return m_meta; };
  const uimax &count() const { return m_meta; };

  template <typename... Input> void at(uimax p_index, Input &&... p_input) {
    assert_debug(p_index < count());
    __at_v2<Input...>{}(*this, p_index, p_input...);
  };

  template <typename... Input> void range(Input... p_ranges) {
    __range<0, Input...>{}(*this, p_ranges...);
  };

private:
  struct table_span_allocate {
    void operator()(table_span_v2 &thiz, uimax p_count) {
      allocate_col<0>{}(thiz, p_count);
      thiz.count() = p_count;
    };

  private:
    template <ui8 Col> struct allocate_col {
      void operator()(table_span_v2 &thiz, uimax p_count) {
        if constexpr (Col < COL_COUNT) {
          auto &l_col = thiz.cols().template col<Col>();
          using T = typename ::traits::remove_ptr_ref<decltype(l_col)>::type;
          l_col = (T *)malloc_free_functions::malloc(p_count * sizeof(T));
          allocate_col<Col + 1>{}(thiz, p_count);
        };
      };
    };
  };

  struct table_span_free {
    void operator()(table_span_v2 &thiz) { free_col<0>{}(thiz); };

  private:
    template <ui8 Col> struct free_col {
      void operator()(table_span_v2 &thiz) {
        if constexpr (Col < COL_COUNT) {
          auto &l_col = thiz.cols().template col<Col>();
          malloc_free_functions::free(l_col);
          free_col<Col + 1>{}(thiz);
        }
      };
    };
  };

  template <typename InputFirst, typename... Input> struct __at_v2 {
    void operator()(table_span_v2 &thiz, uimax p_index, InputFirst &&p_first,
                    Input &... p_input) {

      using ref_t = typename ::traits::remove_ptr_ref<InputFirst>::type;
      p_first.data() = &(thiz.cols().template col<ref_t::COL>())[p_index];
      if constexpr (sizeof...(Input) > 0) {
        __at_v2<Input...>{}(thiz, p_index, p_input...);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input> struct __range {
    void operator()(table_span_v2 &thiz, InputFirst p_first, Input... p_input) {
      if constexpr (!traits::is_none<InputFirst>::value) {
        auto &l_col = thiz.cols().template col<Col>();
        using T = typename ::traits::remove_ptr_ref<decltype(l_col)>::type;
        *p_first = container::range<T>::make(l_col, thiz.count());
      }
      if constexpr (sizeof...(Input) > 0) {
        __range<Col + 1, Input...>{}(thiz, p_input...);
      }
    };
  };

  struct table_span_realloc {
    void operator()(table_span_v2 &thiz, uimax p_new_count) {
      realloc_col<0>{}(thiz, p_new_count);
      thiz.count() = p_new_count;
    };

  private:
    template <ui8 Col> struct realloc_col {
      void operator()(table_span_v2 &thiz, uimax p_new_count) {
        if constexpr (Col < COL_COUNT) {
          auto &l_col = thiz.cols().template col<Col>();
          using T = typename ::traits::remove_ptr_ref<decltype(l_col)>::type;
          l_col = (T *)malloc_free_functions::realloc(l_col,
                                                      sizeof(T) * p_new_count);
          realloc_col<Col + 1>{}(thiz, p_new_count);
        }
      };
    };
  };
};

template <typename... Types> struct table_vector_v2 {
  container::vector_intrusive m_meta;
  details::cols<Types...> m_cols;
  static constexpr ui8 COL_COUNT = details::cols<Types...>::COL_COUNT;

  uimax &count() { return m_meta.m_count; };
  details::cols<Types...> &cols() { return m_cols; };

  void allocate(uimax p_capacity) {
    m_meta.allocate(p_capacity);
    table_vector_allocate<0>{}(*this, p_capacity);
  };

  void free() { table_vector_free<0>{}(*this); };

  template <typename... Input> void push_back(const Input &... p_input) {
    if (m_meta.add_realloc(1)) {
      __realloc<0>{}(*this);
    }
    table_vector_set_value<0, const Input &...>{}(*this, p_input...);
  };

  void remove_at(uimax p_index) {
    assert_debug(p_index < m_meta.m_count && m_meta.m_count > 0);
    if (p_index < m_meta.m_count - 1) {
      table_vector_memmove_up<0>{}(*this, p_index + 1, 1, 1);
    }

    m_meta.m_count -= 1;
  };

  template <typename... Input> void at(uimax p_index, Input... p_input) {
    assert_debug(p_index < count());
    __at<0, Input...>{}(*this, p_index, p_input...);
  };

private:
  template <ui8 Col> struct table_vector_allocate {
    void operator()(table_vector_v2 &thiz, uimax p_capacity) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        using T = typename ::traits::remove_ptr_ref<decltype(l_col)>::type;
        l_col = (T *)sys::malloc(p_capacity * sizeof(T));
        table_vector_allocate<Col + 1>{}(thiz, p_capacity);
      }
    };
  };

  template <ui8 Col> struct table_vector_free {
    void operator()(table_vector_v2 &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        sys::free(l_col);
        table_vector_free<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col> struct __realloc {
    void operator()(table_vector_v2 &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        using T = typename ::traits::remove_ptr_ref<decltype(l_col)>::type;
        l_col = (T *)sys::realloc(l_col, thiz.m_meta.m_capacity * sizeof(T));
        __realloc<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input>
  struct table_vector_set_value {

    void operator()(table_vector_v2 &thiz, const InputFirst &p_first,
                    const Input &... p_input) {

      if constexpr (!traits::is_none<InputFirst>::value) {
        thiz.cols().template col<Col>()[thiz.m_meta.m_count - 1] = p_first;
      }

      if constexpr (Col + 1 < COL_COUNT) {
        table_vector_set_value<Col + 1, const Input &...>{}(thiz, p_input...);
      }
    };
  };

  template <ui8 Col> struct table_vector_memmove_up {
    void operator()(table_vector_v2 &thiz, uimax p_break_index,
                    uimax p_move_delta, uimax p_chunk_count) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        sys::memmove_up_t(l_col, p_break_index, p_move_delta, p_chunk_count);
        table_vector_memmove_up<Col + 1>{}(thiz, p_break_index, p_move_delta,
                                           p_chunk_count);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input> struct __at {
    void operator()(table_vector_v2 &thiz, uimax p_index, InputFirst p_first,
                    Input... p_input) {
      if constexpr (!traits::is_none<InputFirst>::value) {
        *p_first = &(thiz.cols().template col<Col>())[p_index];
      }
      if constexpr (sizeof...(Input) > 0) {
        __at<Col + 1, Input...>{}(thiz, p_index, p_input...);
      }
    };
  };
};

}; // namespace orm