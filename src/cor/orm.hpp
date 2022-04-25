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
      p_table.m_col_0, l_meta.m_capacity * sizeof(p_table.m_col_0));
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
uimax __push_back_heap_bytes(TableType &p_table, uimax p_size) {
  container::heap_intrusive &l_meta = p_table.m_meta;
  uimax l_chunk_index = l_meta.find_next_chunk(p_size);
  if (l_meta.m_state == container::heap_intrusive::state::NewChunkPushed) {
    l_meta.clear_state();
    p_table.m_col_0.push_back(sys::malloc(l_meta.m_single_page_capacity));
  }
  return l_chunk_index;
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
  };

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
  };

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
  };

}; // namespace orm