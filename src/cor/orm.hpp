#pragma once

#include <cor/container.hpp>
#include <cor/cor.hpp>
#include <sys/sys.hpp>

namespace orm {

enum class table_memory_layout {
  UNDEFINED = 0,
  POOL = 1,
  POOL_FIXED = 2,
  VECTOR = 3,
  HEAP = 4
};

namespace details {

template <table_memory_layout MemoryLayout> struct table_meta {};

template <typename TableType, int N>
static constexpr inline bool table_loop_next() {
  return N < TableType::COL_COUNT - 1;
};

template <typename TableType> struct table_allocate {

private:
  template <int N> struct __allocate_recursive;

public:
  table_allocate(TableType &p_table, uimax_t p_capacity) {
    p_table.meta().allocate(p_capacity);
    __allocate_recursive<0>{}(p_table);
  };

private:
  template <int Col> struct __allocate_recursive {
    void operator()(TableType &p_table) const {
      p_table.template col<Col>().allocate(p_table);
      if constexpr (table_loop_next<TableType, Col>()) {
        __allocate_recursive<Col + 1>{}(p_table);
      }
    };
  };
};

template <typename TableType> struct table_free {

public:
  table_free(TableType &p_table) {
    p_table.meta().free();
    __free_recursive<0>{}(p_table);
  };

private:
  template <int Col> struct __free_recursive {
    void operator()(TableType &p_table) const {
      using element_type = decltype(p_table.template col_type<Col>());
      p_table.template col<Col>().free(p_table);
      if constexpr (table_loop_next<TableType, Col>()) {
        __free_recursive<Col + 1>{}(p_table);
      }
    };
  };
};

template <typename TableType> struct table_realloc_cols {
  void operator()(TableType &p_table) {
    __table_resize_recursive<0>{}(p_table);
  };

private:
  template <int Col> struct __table_resize_recursive {
    void operator()(TableType &p_table) const {
      p_table.template col<Col>().realloc(p_table);
      if constexpr (table_loop_next<TableType, Col>()) {
        __table_resize_recursive<Col + 1>{}(p_table);
      }
    };
  };
};

// TODO -> have specialization ?
template <typename TableType, table_memory_layout MemoryLayout,
          typename... Types>
struct table_push_back {
  uimax_t m_index;
  table_push_back(TableType &p_table, const Types &... p_elements) {
    m_index = p_table.meta().next_free_element();
    if (m_index == -1) {
      if (p_table.meta().try_resize_for_space(1)) {
        table_realloc_cols<TableType>{}(p_table);
        m_index = p_table.meta().next_free_element();
      }
    };
    assert_debug(m_index != -1);
    __table_push_back_col<0, Types...>{}(p_table, m_index, p_elements...);
  };

private:
  template <int Col, typename Type, typename... LocalTypes>
  struct __table_push_back_col {
    void operator()(TableType &p_table, uimax_t p_index, const Type &p_element,
                    const LocalTypes &... p_next) {
      p_table.template col<Col>().at(p_index) = p_element;
      if constexpr (table_loop_next<TableType, Col>()) {
        __table_push_back_col<Col + 1, LocalTypes...>{}(p_table, p_index,
                                                        p_next...);
      }
    };
  };
};

template <typename TableType> struct table_remove_at {
  table_remove_at(TableType &p_table, uimax_t p_index) {
    p_table.meta().remove_at(p_index);
    __table_remove_at_recursive<0>{}(p_table, p_index);
  };

private:
  template <int Col> struct __table_remove_at_recursive {
    void operator()(TableType &p_table, uimax_t p_index) const {
      p_table.template col<Col>().remove_at(p_table, p_index);
      if constexpr (table_loop_next<TableType, Col>()) {
        __table_remove_at_recursive<Col + 1>{}(p_table, p_index);
      }
    };
  };
};
// orm::details::table_push_back<orm::table<orm::table_col_types<int, int>,
// orm::table_memory_layout::HEAP>, orm::table_memory_layout::HEAP, int,
// int>::table_push_back
}; // namespace details

namespace details {

template <typename T, table_memory_layout MemoryLayout> struct __table_iterator;

}; // namespace details

template <typename TableType, int Col> struct table_iterator {
  using element_type = decltype(TableType::template col_type<Col>());
  using iternal_it_type =
      details::__table_iterator<element_type,
                                TableType::meta_type::MEMORY_LAYOUT>;
  iternal_it_type m_internal_it;

  table_iterator(TableType &p_table)
      : m_internal_it(
            iternal_it_type::template make<TableType, Col>(p_table)){};
  uimax_t &count() { return m_internal_it.count(); };
  ui8_t next() { return m_internal_it.next(); };
  element_type &value() { return m_internal_it.value(); };
};

template <typename T, table_memory_layout MemoryLayout> struct col;
template <typename... Types> struct table_col_types;

template <typename ColTypes, table_memory_layout MemoryLayout,
          int Col = ColTypes::COL_COUNT>
struct table_cols;

template <typename ColTypes,
          table_memory_layout MemoryLayout = table_memory_layout::POOL>
struct table {
  using meta_type = details::table_meta<MemoryLayout>;
  static constexpr ui8_t COL_COUNT = ColTypes::COL_COUNT;
  meta_type m_meta;
  table_cols<ColTypes, MemoryLayout> m_cols;

  meta_type &meta() { return m_meta; };
  const meta_type &meta() const { return m_meta; };

  void allocate(uimax_t p_capacity) {
    details::table_allocate<table>(*this, p_capacity);
  };

  void free() { details::table_free<table>(*this); };

  template <typename... Types> uimax_t push_back(const Types &... p_elements) {
    return details::table_push_back<table, MemoryLayout, Types...>(
               *this, p_elements...)
        .m_index;
  };

  void remove_at(uimax_t p_index) {
    details::table_remove_at<table>(*this, p_index);
  };

  template <int N> static auto col_type() {
    return ColTypes::template col_element_type<N>();
  };
  template <int N> auto &col() { return m_cols.template col<N>(); };

  template <int N> auto iter() { return table_iterator<table, N>(*this); };

  template <int N> auto at(uimax_t p_index) -> decltype(col_type<N>()) & {
    return m_cols.template col<N>().at(p_index);
  };

  template <int N>
  uimax_t get_fixed_index(const decltype(col_type<N>()) *p_ptr) {
    static_assert(MemoryLayout == table_memory_layout::POOL_FIXED, "");
    return m_cols.template col<N>().get_fixed_index(p_ptr);
  };

  ui8_t has_allocated_elements() const {
    return meta().has_allocated_elements();
  };
};

namespace details {

template <typename DbType> struct db_allocate {
  template <typename... Capacities>
  db_allocate(DbType &p_db, const Capacities &... p_capacities) {
    __db_allocate<0, Capacities...>{}(p_db, p_capacities...);
  };

private:
  template <int Tab, typename Capacity, typename... NextCapacities>
  struct __db_allocate {
    void operator()(DbType &p_db, const Capacity &p_capacity,
                    const NextCapacities &... p_next_capacities) {
      auto &l_table = p_db.template table<Tab>();
      l_table.allocate(p_capacity);
      if constexpr (Tab < DbType::TABLE_COUNT - 1) {
        __db_allocate<Tab + 1, NextCapacities...>{}(p_db, p_next_capacities...);
      }
    };
  };
};

template <typename DbType> struct db_free {
  db_free(DbType &p_db) { __db_free<0>{}(p_db); };

private:
  template <int Tab> struct __db_free {
    void operator()(DbType &p_db) {
      auto &l_table = p_db.template table<Tab>();
      l_table.free();
      if constexpr (Tab < DbType::TABLE_COUNT - 1) {
        __db_free<Tab + 1>{}(p_db);
      }
    };
  };
};

}; // namespace details

template <typename... DbTableTypes> struct db_table_types;
template <typename DbTableTypes, int TableCount = DbTableTypes::TABLE_COUNT>
struct db_tables;

template <typename DbTableTypes> struct db {
  static constexpr int TABLE_COUNT = DbTableTypes::TABLE_COUNT;
  db_tables<DbTableTypes> m_tables;
  template <int Tab> auto &table() { return m_tables.template table<Tab>(); };
  template <int Tab> static auto table_type() {
    return DbTableTypes::template table_type<Tab>();
  };

  template <typename... Capacities> void allocate(Capacities... p_capacities) {
    details::db_allocate<db>(*this, p_capacities...);
  };
  void free() { details::db_free<db>(*this); };

  template <int Tab, typename... Types>
  uimax_t push_back(const Types &... p_types) {
    return table<Tab>().push_back(p_types...);
  };

  template <int Tab, int Col> auto &at(uimax_t p_index) {
    return table<Tab>().template at<Col>(p_index);
  };

  template <int Tab> void remove_at(uimax_t p_index) {
    table<Tab>().remove_at(p_index);
  };

  template <int Tab, int N, typename Ptr>
  uimax_t get_fixed_index(const Ptr *p_ptr) {
    return table<Tab>().template get_fixed_index<N>(p_ptr);
  };
};

//////////////////// POOL SPECIALIZATION ////////////////////

template <typename T> struct col<T, table_memory_layout::POOL> {
  T *m_data;
  T *&data() { return m_data; };
  T &at(uimax_t p_index) { return m_data[p_index]; };

  template <typename TableType> void allocate(TableType &p_table) {
    m_data = (T *)sys::malloc(p_table.meta().m_capacity * sizeof(T));
  };

  template <typename TableType> void free(TableType &p_table) {
    sys::free(m_data);
  };

  template <typename TableType> void realloc(TableType &p_table) {
    auto l_new_byte_size = p_table.meta().m_capacity * sizeof(T);
    m_data = (T *)sys::realloc(m_data, l_new_byte_size);
  };

  template <typename TableType>
  void remove_at(TableType &p_table, uimax_t p_index){};
};

namespace details {
template <> struct table_meta<table_memory_layout::POOL> {

  static const table_memory_layout MEMORY_LAYOUT = table_memory_layout::POOL;

  uimax_t m_capacity;
  uimax_t m_count;

  container::vector<uimax_t> m_free_elements;

  void allocate(uimax_t p_capacity) {
    m_capacity = p_capacity;
    m_count = 0;
    m_free_elements.allocate(0);
  };

  void free() { m_free_elements.free(); };

  uimax_t next_free_element() {
    if (m_free_elements.count() > 0) {
      auto l_index = m_free_elements.at(m_free_elements.count() - 1);
      m_free_elements.pop_back();
      return l_index;
    }
    if (m_count < m_capacity) {
      m_count += 1;
      return m_count - 1;
    }
    return -1;
  };

  ui8_t try_resize_for_space(uimax_t p_delta) {
    if (p_delta > 0) {
      uimax_t l_new_capacity = m_capacity + p_delta;
      auto l_calculated_new_capacity = m_capacity;
      if (l_calculated_new_capacity == 0) {
        l_calculated_new_capacity = 1;
      };
      while (l_calculated_new_capacity < l_new_capacity) {
        l_calculated_new_capacity *= 2;
      }
      m_capacity = l_calculated_new_capacity;
      return 1;
    }
    return 0;
  };

  void remove_at(uimax_t p_index) {
    assert_debug(__is_element_allocated(p_index));
    m_free_elements.push_back(p_index);
  };

  ui8_t has_allocated_elements() const {
    return m_free_elements.count() != m_count;
  };

private:
  ui8_t __is_element_allocated(uimax_t p_index) const {
    for (auto i = 0; i < m_free_elements.count(); ++i) {
      if (m_free_elements.at(i) == p_index) {
        return 0;
      }
    }
    return 1;
  };
};

}; // namespace details

template <typename T> struct col<T, table_memory_layout::VECTOR> {
  T *m_data;
  T *&data() { return m_data; };
  T &at(uimax_t p_index) { return m_data[p_index]; };

  template <typename TableType> void allocate(TableType &p_table) {
    m_data = (T *)sys::malloc(p_table.meta().m_capacity * sizeof(T));
  };

  template <typename TableType> void free(TableType &p_table) {
    sys::free(m_data);
  };

  template <typename TableType> void realloc(TableType &p_table) {
    auto l_new_byte_size = p_table.meta().m_capacity * sizeof(T);
    m_data = (T *)sys::realloc(m_data, l_new_byte_size);
  };

  template <typename TableType>
  void remove_at(TableType &p_table, uimax_t p_index) {
    if (p_index < p_table.meta().m_capacity) {
      sys::memmove_up(m_data, p_index + 1, 1, 1);
    }
  };
};

namespace details {
template <> struct table_meta<table_memory_layout::VECTOR> {

  static const table_memory_layout MEMORY_LAYOUT = table_memory_layout::VECTOR;

  uimax_t m_capacity;
  uimax_t m_count;

  uimax_t &count() { return m_count; };

  void allocate(uimax_t p_capacity) {
    m_capacity = p_capacity;
    m_count = 0;
  };

  void free(){};

  uimax_t next_free_element() {
    if (m_count < m_capacity) {
      m_count += 1;
      return m_count - 1;
    }
    return -1;
  };

  void remove_at(uimax_t p_index) { m_count -= 1; };

  ui8_t try_resize_for_space(uimax_t p_delta) {
    if (p_delta > 0) {
      uimax_t l_new_capacity = m_capacity + p_delta;
      auto l_calculated_new_capacity = m_capacity;
      if (l_calculated_new_capacity == 0) {
        l_calculated_new_capacity = 1;
      };
      while (l_calculated_new_capacity < l_new_capacity) {
        l_calculated_new_capacity *= 2;
      }
      m_capacity = l_calculated_new_capacity;
      return 1;
    }
    return 0;
  };

  ui8_t has_allocated_elements() const { return m_count > 0; };
};

template <typename T> struct __table_iterator<T, table_memory_layout::VECTOR> {
  T *&m_begin;
  uimax_t &m_count;
  uimax_t m_index;

  __table_iterator(T *&p_begin, uimax_t &p_count, uimax_t p_index)
      : m_begin(p_begin), m_count(p_count), m_index(p_index){};

  template <typename TableType, int Col>
  static __table_iterator make(TableType &p_table) {
    return __table_iterator(p_table.template col<Col>().data(),
                            p_table.meta().count(), -1);
  };

  uimax_t &count() { return m_count; };

  ui8_t next() {
    m_index += 1;
    return m_index < m_count;
  };

  T &value() { return m_begin[m_index]; };
};

}; // namespace details

template <typename T> struct col<T, table_memory_layout::POOL_FIXED> {
  container::vector<T *> m_data;
  uimax_t m_chunk_count;
  details::table_meta<table_memory_layout::POOL_FIXED> *m_meta;

  template <typename TableType> void allocate(TableType &p_table) {
    m_data.allocate(0);
    m_chunk_count = p_table.meta().m_chunk_count;
  };

  template <typename TableType> void free(TableType &p_table) {
    for (auto i = 0; i < m_data.count(); ++i) {
      sys::free(m_data.at(i));
    }
    m_data.free();
  };

  template <typename TableType> void realloc(TableType &p_table) {
    assert_debug(p_table.meta().m_debug_chunks_just_expanded);

    auto l_chunk_count = p_table.meta().m_chunk_count;
    T *l_chunk_data = (T *)sys::malloc(sizeof(T) * l_chunk_count);
    m_data.push_back(l_chunk_data);
  };

  template <typename TableType>
  void remove_at(TableType &p_table_type, uimax_t p_index){};

  T &at(uimax_t p_index) {
    uimax_t l_chunk_index = p_index / m_chunk_count;
    uimax_t l_index = p_index - l_chunk_index;
    return m_data.at(l_chunk_index)[l_index];
  };

  uimax_t get_fixed_index(const T *p_ptr) {
    uimax_t l_index = 0;
    for (auto i = 0; i < m_data.count(); ++i) {
      const T *l_begin = m_data.at(i);
      const T *l_end = l_begin + m_chunk_count;
      if (p_ptr >= l_begin && p_ptr < l_end) {
        l_index += p_ptr - l_begin;
        return l_index;
      }
      l_index += m_chunk_count;
    }
    return -1;
  };
};

namespace details {

// TODO -> move logic to the container ?
template <> struct table_meta<table_memory_layout::POOL_FIXED> {
  static const table_memory_layout MEMORY_LAYOUT =
      table_memory_layout::POOL_FIXED;

  struct chunk {
    container::vector<uimax_t> m_free_elements;

    void allocate(uimax_t p_capacity) {
      m_free_elements.allocate(p_capacity);
      m_free_elements.m_count = p_capacity;
      for (auto i = 0; i < m_free_elements.count(); ++i) {
        auto l_index = m_free_elements.count() - 1 - i;
        m_free_elements.at(l_index) = l_index;
      }
    };

    void free() { m_free_elements.free(); };
  };

  container::vector<chunk> m_chunks;
  uimax_t m_chunk_count;
  ui8_t m_debug_chunks_just_expanded;

  void allocate(uimax_t p_capacity) {
    m_chunks.allocate(0);
    m_chunk_count = p_capacity;
    block_debug(m_debug_chunks_just_expanded = 0);
  };

  void free() {
    for (auto i = 0; i < m_chunks.count(); ++i) {
      m_chunks.at(i).free();
    }
    m_chunks.free();
  };

  uimax_t next_free_element() {
    uimax_t l_index = 0;
    for (auto i = 0; i < m_chunks.count(); ++i) {
      auto &l_chunks = m_chunks.at(i);
      if (l_chunks.m_free_elements.count() > 0) {
        l_index +=
            l_chunks.m_free_elements.at(l_chunks.m_free_elements.count() - 1);
        l_chunks.m_free_elements.pop_back();
        return l_index;
      }
      l_index += m_chunk_count;
    }
    return -1;
  };

  ui8_t try_resize_for_space(uimax_t p_delta) {
    block_debug(m_debug_chunks_just_expanded = 0);
    if (p_delta > 0) {
      assert_debug(m_chunk_count >= p_delta);
      chunk l_chunk;
      l_chunk.allocate(m_chunk_count);
      m_chunks.push_back(l_chunk);
      block_debug(m_debug_chunks_just_expanded = 1);
      return 1;
    }
    return 0;
  };

  void remove_at(uimax_t p_index) {
    uimax_t l_chunk_index, l_index;
    get_index(p_index, l_chunk_index, l_index);
    m_chunks.at(l_chunk_index).m_free_elements.push_back(l_index);
  };

  void get_index(uimax_t p_index, uimax_t &out_chunk_index,
                 uimax_t &out_index) {
    out_chunk_index = p_index / m_chunk_count;
    out_index = p_index - out_chunk_index;
  };

  ui8_t has_allocated_elements() const {
    for (auto i = 0; i < m_chunks.count(); ++i) {
      if (m_chunks.at(i).m_free_elements.count() != m_chunk_count) {
        return 1;
      }
    }
    return 0;
  };
};

}; // namespace details

template <typename T> struct col<T, table_memory_layout::HEAP> {
  T *m_data;

  template <typename TableType> void allocate(TableType &p_table) {
    m_data =
        (T *)sys::malloc(sizeof(T) * p_table.meta().m_heap_intrusive.count());
  };

  template <typename TableType> void free(TableType &p_table) {
    sys::free(m_data);
  };

  template <typename TableType> void realloc(TableType &p_table) {
    auto l_new_byte_size = p_table.meta().m_heap_intrusive.count() * sizeof(T);
    m_data = (T *)sys::realloc(m_data, l_new_byte_size);
  };

  T &at(uimax_t p_index) { return m_data[p_index]; };

  template <typename TableType>
  void remove_at(TableType &p_table, uimax_t p_index){};
};

namespace details {
template <> struct table_meta<table_memory_layout::HEAP> {

  static const table_memory_layout MEMORY_LAYOUT = table_memory_layout::HEAP;

  container::heap_intrusive m_heap_intrusive;

  void allocate(uimax_t p_capacity) { m_heap_intrusive.allocate(p_capacity); };

  void free() { m_heap_intrusive.free(); };

  uimax_t next_free_element(uimax_t p_size) {
    return m_heap_intrusive.find_next_chunk(p_size);
  };

  void remove_at(uimax_t p_index) { m_heap_intrusive.free(p_index); };
};

template <typename TableType, typename... Types>
struct table_push_back<TableType, table_memory_layout::HEAP, Types...> {
  uimax_t m_index;
  table_push_back(TableType &p_table, const Types &... p_elements) {
    table_meta<table_memory_layout::HEAP> &l_meta = p_table.meta();
    uimax_t l_free_chunk = l_meta.next_free_element(1);
    assert_debug(l_free_chunk != -1);
    m_index = l_meta.m_heap_intrusive.push_found_chunk(1, l_free_chunk);
    container::heap_chunk &l_chunk =
        l_meta.m_heap_intrusive.m_allocated_chunk.at(m_index);
    __table_push_back_col<0>{}(p_table, l_chunk, p_elements...);
    l_meta.m_heap_intrusive.clear_state();
  };

private:
  template <int Col> struct __table_push_back_col {
    template <typename Type, typename... LocalTypes>
    void operator()(TableType &p_table, const container::heap_chunk &p_chunk,
                    const Type &p_element, const LocalTypes &... p_next) const {
      if (p_table.meta().m_heap_intrusive.m_state ==
          container::heap_intrusive::state::NEW_CHUNK_PUSHED) {
        p_table.template col<Col>().realloc(p_table);
      }
      p_table.template col<Col>().at(p_chunk.m_begin) = p_element;
      if constexpr (table_loop_next<TableType, Col>()) {
        __table_push_back_col<Col + 1>{}(p_table, p_chunk, p_next...);
      }
    };
  };
};

}; // namespace details

#pragma region TRIVIAL SPECIALIZATIONS

#define table_col_types_col_element_type(p_number)                             \
  template <> static FORCE_INLINE auto col_element_type<p_number>() {          \
    return Type##p_number{};                                                   \
  };

template <typename Type0> struct table_col_types<Type0> {
  static constexpr ui8_t COL_COUNT = 1;
  template <int N> static auto col_element_type();
  table_col_types_col_element_type(0);
};

template <typename Type0, typename Type1> struct table_col_types<Type0, Type1> {
  static constexpr ui8_t COL_COUNT = 2;
  template <int N> static auto col_element_type();
  table_col_types_col_element_type(0);
  table_col_types_col_element_type(1);
};

template <typename Type0, typename Type1, typename Type2>
struct table_col_types<Type0, Type1, Type2> {
  static constexpr ui8_t COL_COUNT = 3;
  template <int N> static auto col_element_type();
  table_col_types_col_element_type(0);
  table_col_types_col_element_type(1);
  table_col_types_col_element_type(2);
};

template <typename Type0, typename Type1, typename Type2, typename Type3>
struct table_col_types<Type0, Type1, Type2, Type3> {
  static constexpr ui8_t COL_COUNT = 4;
  template <int N> static auto col_element_type();
  table_col_types_col_element_type(0);
  table_col_types_col_element_type(1);
  table_col_types_col_element_type(2);
  table_col_types_col_element_type(3);
};

#undef table_col_types_col_element_type

template <typename ColTypes, table_memory_layout MemoryLayout>
struct table_cols<ColTypes, MemoryLayout, 1> {
  col<decltype(ColTypes::template col_element_type<0>()), MemoryLayout> m_col_0;

  template <int N> auto &col();
  template <> FORCE_INLINE auto &col<0>() { return m_col_0; };
};

template <typename ColTypes, table_memory_layout MemoryLayout>
struct table_cols<ColTypes, MemoryLayout, 2> {
  col<decltype(ColTypes::template col_element_type<0>()), MemoryLayout> m_col_0;
  col<decltype(ColTypes::template col_element_type<1>()), MemoryLayout> m_col_1;

  template <int N> auto &col();
  template <> FORCE_INLINE auto &col<0>() { return m_col_0; };
  template <> FORCE_INLINE auto &col<1>() { return m_col_1; };
};

template <typename ColTypes, table_memory_layout MemoryLayout>
struct table_cols<ColTypes, MemoryLayout, 3> {
  col<decltype(ColTypes::template col_element_type<0>()), MemoryLayout> m_col_0;
  col<decltype(ColTypes::template col_element_type<1>()), MemoryLayout> m_col_1;
  col<decltype(ColTypes::template col_element_type<2>()), MemoryLayout> m_col_2;

  template <int N> auto &col();
  template <> FORCE_INLINE auto &col<0>() { return m_col_0; };
  template <> FORCE_INLINE auto &col<1>() { return m_col_1; };
  template <> FORCE_INLINE auto &col<2>() { return m_col_2; };
};
template <typename ColTypes, table_memory_layout MemoryLayout>
struct table_cols<ColTypes, MemoryLayout, 4> {
  col<decltype(ColTypes::template col_element_type<0>()), MemoryLayout> m_col_0;
  col<decltype(ColTypes::template col_element_type<1>()), MemoryLayout> m_col_1;
  col<decltype(ColTypes::template col_element_type<2>()), MemoryLayout> m_col_2;
  col<decltype(ColTypes::template col_element_type<3>()), MemoryLayout> m_col_3;

  template <int N> auto &col();
  template <> FORCE_INLINE auto &col<0>() { return m_col_0; };
  template <> FORCE_INLINE auto &col<1>() { return m_col_1; };
  template <> FORCE_INLINE auto &col<2>() { return m_col_2; };
  template <> FORCE_INLINE auto &col<3>() { return m_col_3; };
};

template <typename Table0> struct db_table_types<Table0> {
  static constexpr int TABLE_COUNT = 1;
  template <int N> static auto table_type();
  template <> FORCE_INLINE static auto table_type<0>() { return Table0{}; };
};

template <typename Table0, typename Table1>
struct db_table_types<Table0, Table1> {
  static constexpr int TABLE_COUNT = 2;
  template <int N> static auto table_type();
  template <> FORCE_INLINE static auto table_type<0>() { return Table0{}; };
  template <> FORCE_INLINE static auto table_type<1>() { return Table1{}; };
};

template <typename Table0, typename Table1, typename Table2>
struct db_table_types<Table0, Table1, Table2> {
  static constexpr int TABLE_COUNT = 3;
  template <int N> static auto table_type();
  template <> FORCE_INLINE static auto table_type<0>() { return Table0{}; };
  template <> FORCE_INLINE static auto table_type<1>() { return Table1{}; };
  template <> FORCE_INLINE static auto table_type<2>() { return Table2{}; };
};

template <typename Table0, typename Table1, typename Table2, typename Table3>
struct db_table_types<Table0, Table1, Table2, Table3> {
  static constexpr int TABLE_COUNT = 4;
  template <int N> static auto table_type();
  template <> FORCE_INLINE static auto table_type<0>() { return Table0{}; };
  template <> FORCE_INLINE static auto table_type<1>() { return Table1{}; };
  template <> FORCE_INLINE static auto table_type<2>() { return Table2{}; };
  template <> FORCE_INLINE static auto table_type<3>() { return Table3{}; };
};

template <typename Table0, typename Table1, typename Table2, typename Table3,
          typename Table4>
struct db_table_types<Table0, Table1, Table2, Table3, Table4> {
  static constexpr int TABLE_COUNT = 5;
  template <int N> static auto table_type();
  template <> FORCE_INLINE static auto table_type<0>() { return Table0{}; };
  template <> FORCE_INLINE static auto table_type<1>() { return Table1{}; };
  template <> FORCE_INLINE static auto table_type<2>() { return Table2{}; };
  template <> FORCE_INLINE static auto table_type<3>() { return Table3{}; };
  template <> FORCE_INLINE static auto table_type<4>() { return Table4{}; };
};

template <typename DbTableTypes> struct db_tables<DbTableTypes, 1> {
  decltype(DbTableTypes::template table_type<0>()) m_table_0;

  template <int N> auto &table();
  template <> FORCE_INLINE auto &table<0>() { return m_table_0; };
};

template <typename DbTableTypes> struct db_tables<DbTableTypes, 2> {
  decltype(DbTableTypes::template table_type<0>()) m_table_0;
  decltype(DbTableTypes::template table_type<1>()) m_table_1;

  template <int N> auto &table();
  template <> FORCE_INLINE auto &table<0>() { return m_table_0; };
  template <> FORCE_INLINE auto &table<1>() { return m_table_1; };
};

template <typename DbTableTypes> struct db_tables<DbTableTypes, 3> {
  decltype(DbTableTypes::template table_type<0>()) m_table_0;
  decltype(DbTableTypes::template table_type<1>()) m_table_1;
  decltype(DbTableTypes::template table_type<2>()) m_table_2;

  template <int N> auto &table();
  template <> FORCE_INLINE auto &table<0>() { return m_table_0; };
  template <> FORCE_INLINE auto &table<1>() { return m_table_1; };
  template <> FORCE_INLINE auto &table<2>() { return m_table_2; };
};

template <typename DbTableTypes> struct db_tables<DbTableTypes, 4> {
  decltype(DbTableTypes::template table_type<0>()) m_table_0;
  decltype(DbTableTypes::template table_type<1>()) m_table_1;
  decltype(DbTableTypes::template table_type<2>()) m_table_2;
  decltype(DbTableTypes::template table_type<3>()) m_table_3;

  template <int N> auto &table();
  template <> FORCE_INLINE auto &table<0>() { return m_table_0; };
  template <> FORCE_INLINE auto &table<1>() { return m_table_1; };
  template <> FORCE_INLINE auto &table<2>() { return m_table_2; };
  template <> FORCE_INLINE auto &table<3>() { return m_table_3; };
};

template <typename DbTableTypes> struct db_tables<DbTableTypes, 5> {
  decltype(DbTableTypes::template table_type<0>()) m_table_0;
  decltype(DbTableTypes::template table_type<1>()) m_table_1;
  decltype(DbTableTypes::template table_type<2>()) m_table_2;
  decltype(DbTableTypes::template table_type<3>()) m_table_3;
  decltype(DbTableTypes::template table_type<4>()) m_table_4;

  template <int N> auto &table();
  template <> FORCE_INLINE auto &table<0>() { return m_table_0; };
  template <> FORCE_INLINE auto &table<1>() { return m_table_1; };
  template <> FORCE_INLINE auto &table<2>() { return m_table_2; };
  template <> FORCE_INLINE auto &table<3>() { return m_table_3; };
  template <> FORCE_INLINE auto &table<4>() { return m_table_4; };
};
#pragma endregion TRIVIAL SPECIALIZATIONS

}; // namespace orm