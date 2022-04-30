#pragma once

#include <cor/container.hpp>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace orm {

struct none {};

struct row {
  uimax idx;
};

namespace details {

template <typename TableType>
bool __has_allocated_elements_vector(TableType &p_table) {
  container::vector_intrusive &l_intrusive = p_table.m_meta;
  return l_intrusive.m_count > 0;
};

template <typename TableType>
void __allocate_vector_1(TableType &p_table, uimax p_capacity) {
  p_table.m_meta.allocate(p_capacity);
  p_table.m_col_0 = (decltype(p_table.m_col_0))sys::malloc(
      p_table.m_meta.m_capacity * sizeof(*p_table.m_col_0));
};

template <typename TableType> void __free_vector_1(TableType &p_table) {
  sys::free(p_table.m_col_0);
};

template <typename TableType> void __realloc_vector_1(TableType &p_table) {
  p_table.m_col_0 = (decltype(p_table.m_col_0))sys::realloc(
      p_table.m_col_0, p_table.m_meta.m_capacity * sizeof(*p_table.m_col_0));
};

template <typename TableType>
void __push_back_vector_1(TableType &p_table,
                          const typename TableType::type_0 &p_0) {
  container::vector_intrusive &l_intrusive = p_table.m_meta;
  if (l_intrusive.add_realloc(1)) {
    __realloc_vector_1(p_table);
  }
  p_table.m_col_0[l_intrusive.m_count - 1] = p_0;
};

template <typename TableType>
void __remove_at_vector_1(TableType &p_table, uimax p_index) {
  container::vector_intrusive &l_intrusive = p_table.m_meta;
  assert_debug(p_index < l_intrusive.m_count && l_intrusive.m_count > 0);
  if (p_index < l_intrusive.m_count - 1) {
    sys::memmove_up_t(p_table.m_col_0, p_index + 1, 1, 1);
  }

  l_intrusive.m_count -= 1;
};

template <typename TableType> uimax __element_count_vector(TableType &p_table) {
  container::vector_intrusive &l_intrusive = p_table.m_meta;
  return l_intrusive.m_count;
};

template <typename TableType>
void __at_vector_1(TableType &p_table, uimax p_index,
                   typename TableType::type_0 **out_0) {
  assert_debug(p_index < p_table.m_meta.m_count);
  *out_0 = &p_table.m_col_0[p_index];
};

template <typename TableType>
void __allocate_vector_2(TableType &p_table, uimax p_capacity) {
  p_table.m_meta.allocate(p_capacity);
  p_table.m_col_0 = (decltype(p_table.m_col_0))sys::malloc(
      p_table.m_meta.m_capacity * sizeof(*p_table.m_col_0));
  p_table.m_col_1 = (decltype(p_table.m_col_1))sys::malloc(
      p_table.m_meta.m_capacity * sizeof(*p_table.m_col_1));
};

template <typename TableType> void __free_vector_2(TableType &p_table) {
  sys::free(p_table.m_col_0);
  sys::free(p_table.m_col_1);
};

template <typename TableType> void __realloc_vector_2(TableType &p_table) {
  p_table.m_col_0 = (decltype(p_table.m_col_0))sys::realloc(
      p_table.m_col_0, p_table.m_meta.m_capacity * sizeof(*p_table.m_col_0));
  p_table.m_col_1 = (decltype(p_table.m_col_1))sys::realloc(
      p_table.m_col_1, p_table.m_meta.m_capacity * sizeof(*p_table.m_col_1));
};

template <typename TableType>
void __push_back_vector_2(TableType &p_table,
                          const typename TableType::type_0 &p_0,
                          const typename TableType::type_1 &p_1) {
  container::vector_intrusive &l_intrusive = p_table.m_meta;
  if (l_intrusive.add_realloc(1)) {
    __realloc_vector_2(p_table);
  }
  p_table.m_col_0[l_intrusive.m_count - 1] = p_0;
  p_table.m_col_1[l_intrusive.m_count - 1] = p_1;
};

template <typename TableType>
void __remove_at_vector_2(TableType &p_table, uimax p_index) {
  container::vector_intrusive &l_intrusive = p_table.m_meta;
  assert_debug(p_index < l_intrusive.m_count && l_intrusive.m_count > 0);
  if (p_index < l_intrusive.m_count - 1) {
    sys::memmove_up_t(p_table.m_col_0, p_index + 1, 1, 1);
    sys::memmove_up_t(p_table.m_col_1, p_index + 1, 1, 1);
  }

  l_intrusive.m_count -= 1;
};

template <typename TableType>
void __at_vector_2(TableType &p_table, uimax p_index,
                   typename TableType::type_0 **out_0,
                   typename TableType::type_1 **out_1) {
  assert_debug(p_index < p_table.m_meta.m_count);
  *out_0 = &p_table.m_col_0[p_index];
  *out_1 = &p_table.m_col_1[p_index];
};

template <typename TableType>
void __at_vector_2(TableType &p_table, uimax p_index,
                   typename TableType::type_0 **out_0, orm::none) {
  *out_0 = &p_table.m_col_0[p_index];
};

template <typename TableType>
void __at_vector_2(TableType &p_table, uimax p_index, orm::none,
                   typename TableType::type_1 **out_1) {
  *out_1 = &p_table.m_col_1[p_index];
};

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
void __remove_at_pool_1(TableType &p_table, uimax p_index) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  l_meta.free_element(p_index);
};

template <typename TableType>
void __at_pool_1(TableType &p_table, uimax p_index,
                 typename TableType::type_0 **out_0) {
  container::pool_intrusive &l_meta = p_table.m_meta;
  assert_debug(p_index < l_meta.m_count);
  *out_0 = &p_table.m_col_0[p_index];
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

#define table_vector_meta container::vector_intrusive m_meta;

#define table_cols_1(Type0)                                                    \
  using type_0 = Type0;                                                        \
  type_0 *m_col_0;

#define table_define_vector_1                                                  \
  void allocate(uimax p_capacity) {                                            \
    orm::details::__allocate_vector_1(*this, p_capacity);                      \
  };                                                                           \
  void free() { orm::details::__free_vector_1(*this); };                       \
  void push_back(const type_0 &p_0) {                                          \
    orm::details::__push_back_vector_1(*this, p_0);                            \
  };                                                                           \
  void remove_at(uimax p_index) {                                              \
    orm::details::__remove_at_vector_1(*this, p_index);                        \
  };                                                                           \
  void at(uimax p_index, type_0 **out_0) {                                     \
    orm::details::__at_vector_1(*this, p_index, out_0);                        \
  };                                                                           \
  bool has_allocated_elements() {                                              \
    return orm::details::__has_allocated_elements_vector(*this);               \
  };                                                                           \
  uimax element_count() { return orm::details::__element_count_vector(*this); };

#define table_cols_2(Type0, Type1)                                             \
  using type_0 = Type0;                                                        \
  using type_1 = Type1;                                                        \
  type_0 *m_col_0;                                                             \
  type_1 *m_col_1;

#define table_define_vector_2                                                  \
  void allocate(uimax p_capacity) {                                            \
    orm::details::__allocate_vector_2(*this, p_capacity);                      \
  };                                                                           \
  void free() { orm::details::__free_vector_2(*this); };                       \
  void push_back(const type_0 &p_0, const type_1 &p_1) {                       \
    orm::details::__push_back_vector_2(*this, p_0, p_1);                       \
  };                                                                           \
  void remove_at(uimax p_index) {                                              \
    orm::details::__remove_at_vector_2(*this, p_index);                        \
  };                                                                           \
  void at(uimax p_index, type_0 **out_0, type_1 **out_1) {                     \
    orm::details::__at_vector_2(*this, p_index, out_0, out_1);                 \
  };                                                                           \
                                                                               \
  void at(uimax p_index, orm::none p_none, type_1 **out_1) {                   \
    orm::details::__at_vector_2(*this, p_index, p_none, out_1);                \
  };                                                                           \
                                                                               \
  void at(uimax p_index, type_0 **out_0, orm::none p_none) {                   \
    orm::details::__at_vector_2(*this, p_index, out_0, p_none);                \
  };                                                                           \
  bool has_allocated_elements() {                                              \
    return orm::details::__has_allocated_elements_vector(*this);               \
  };                                                                           \
  uimax element_count() { return orm::details::__element_count_vector(*this); };

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
    orm::details::__remove_at_pool_1(*this, p_index);                          \
  };                                                                           \
  bool has_allocated_elements() {                                              \
    return orm::details::__has_allocated_elements_pool(*this);                 \
  };                                                                           \
  void at(uimax p_index, type_0 **out_0) {                                     \
    orm::details::__at_pool_1(*this, p_index, out_0);                          \
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