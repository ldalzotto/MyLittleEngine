
#pragma once

#include <cor/container.hpp>
#include <cor/orm.hpp>

namespace {
struct tests {
  struct entry {
    i32 m_v0, m_v1, m_v2;
    entry() = default;
    entry(i32 p_value) {
      m_v0 = p_value;
      m_v1 = p_value;
      m_v2 = p_value;
    };

    ui8 operator==(i32 p_value) {
      return m_v0 == p_value && m_v1 == p_value && m_v2 == p_value;
    };
  };

  inline static void containers() {

    container::vector<entry> l_vector;
    l_vector.allocate(0);
    {
      for (auto i = 0; i < 10; ++i) {
        l_vector.push_back(entry(i));
      }

      for (auto i = 0; i < 10; ++i) {
        sys::sassert(l_vector.at(i) == i);
      }

      l_vector.remove_at(l_vector.count() - 1);
      l_vector.remove_at(l_vector.count() - 2);
    }
    l_vector.free();
  };

  inline static void relational() {

    struct entry_table {
      table_vector_meta;
      table_cols_2(entry, entry);
      table_define_vector_2;
    } entry_table;

    struct int_table {
      table_vector_meta;
      table_cols_1(i32);
      table_define_vector_1;
    } int_table;

    struct int_pool_table {
      table_pool_meta;
      table_cols_1(i32);
      table_define_pool_1;
    } int_pool_table;

    struct entry_heap_table {
      table_heap_paged_meta;
      table_heap_paged_cols_1(entry);
      table_define_heap_paged_1;
    } entry_heap_table;

    orm::table_one_to_many entry_to_int;

    entry_table.allocate(0);
    int_table.allocate(0);
    int_pool_table.allocate(0);
    entry_heap_table.allocate(148);
    entry_to_int.allocate(0);

    for (auto i = 0; i < 10; ++i) {
      entry_table.push_back(entry(i), entry(i + 1));
      int_table.push_back(i);
    }

    entry *l_1, *l_2;
    entry_table.at(0, &l_1, &l_2);
    int_table::type_0 *l_3;
    int_table.at(0, &l_3);

    auto &l_entry_table_meta = entry_table.m_meta;
    while (l_entry_table_meta.m_count > 0) {
      entry_table.remove_at(0);
    }

    auto &l_int_table_meta = int_table.m_meta;
    while (l_int_table_meta.m_count > 0) {
      int_table.remove_at(0);
    }

    entry_to_int.push_back(0, 1);
    entry_to_int.remove(0, 1);

    int_pool_table.push_back(10);
    int_pool_table.remove_at(0);

    uimax l_chunk = entry_heap_table.push_back(3);
    container::range<entry> l_entries;
    entry_heap_table.at(l_chunk, &l_entries);
    sys::sassert(l_entries.m_count == 3);

    entry_table.free();
    int_table.free();
    int_pool_table.free();
    entry_heap_table.free();
    entry_to_int.free();
  };
};
} // namespace

static inline void test_container() {
  tests::containers();
  tests::relational();
};