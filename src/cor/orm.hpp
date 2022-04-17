#pragma once

#include <cor/container.hpp>
#include <cor/cor.hpp>
#include <sys/sys.hpp>

namespace orm {

enum class table_memory_layout {
  UNDEFINED = 0,
  POOL = 1,
  POOL_FIXED = 2,
  VECTOR = 3
};

namespace details {

template <table_memory_layout MemoryLayout> struct table_meta {};

template <typename TableType> struct table_allocate {

private:
  template <int N> struct __allocate_recursive;

public:
  table_allocate(TableType &p_table, uimax_t p_capacity) {
    p_table.m_meta.allocate(p_capacity);
    __allocate_recursive<0>{}(p_table);
  };

private:
  template <int N> struct __allocate_recursive {
    void operator()(TableType &p_table) {
      using element_type = decltype(p_table.template col_type<N>());
      p_table.template col<N>().data() = (element_type *)sys::malloc(
          p_table.meta().m_capacity * sizeof(element_type));
      if constexpr (N < TableType::COL_COUNT - 1) {
        __allocate_recursive<N + 1>{}(p_table);
      }
    };
  };
};

template <typename TableType> struct table_free {
private:
  template <int N> struct __free_recursive;

public:
  table_free(TableType &p_table) {
    p_table.meta().free();
    __free_recursive<0>{}(p_table);
  };

private:
  template <int N> struct __free_recursive {
    void operator()(TableType &p_table) {
      using element_type = decltype(p_table.template col_type<N>());
      sys::free(p_table.template col<N>().data());
      if constexpr (N < TableType::COL_COUNT - 1) {
        __free_recursive<N + 1>{}(p_table);
      }
    };
  };
};

template <typename TableType> struct table_realloc_cols {
  void operator()(TableType &p_table) { __table_resize<0>{}(p_table); };

private:
  template <int Col> struct __table_resize {
    void operator()(TableType &p_table) {
      using element_type = decltype(p_table.template col_type<Col>());
      auto l_new_byte_size = p_table.meta().m_capacity * sizeof(element_type);
      auto *&l_col_ptr = p_table.template col<Col>().data();
      l_col_ptr = (element_type *)sys::realloc(l_col_ptr, l_new_byte_size);
      if constexpr (Col < TableType::COL_COUNT - 1) {
        __table_resize<Col + 1>{}(p_table);
      }
    };
  };
};

template <typename TableType, typename... Types> struct table_push_back {
  uimax_t m_index;
  table_push_back(TableType &p_table, const Types &... p_elements) {
    m_index = p_table.meta().next_free_element();
    if (m_index == -1) {
      if (p_table.meta().try_resize(p_table.meta().m_capacity + 1)) {
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
      if constexpr (Col < TableType::COL_COUNT - 1) {
        __table_push_back_col<Col + 1, LocalTypes...>{}(p_table, p_index,
                                                        p_next...);
      }
    };
  };
};

template <int Col, table_memory_layout MemoryLayout>
struct table_remove_at_specialized;

template <typename TableType> struct table_remove_at {
  table_remove_at(TableType &p_table, uimax_t p_index) {
    p_table.meta().remove_at(p_index);
    __table_before_remove<0>{}(p_table, p_index);
    table_remove_at_specialized<0, TableType::meta_type::MEMORY_LAYOUT>{}(
        p_table, p_index);
  };

private:
  template <int Col> struct __table_before_remove {
    void operator()(TableType &p_table, uimax_t p_index) {
      p_table.m_hooks.template before_remove<TableType, Col>(
          p_table, p_index, p_table.template col<Col>().at(p_index));
      if constexpr (Col < TableType::COL_COUNT - 1) {
        __table_before_remove<Col + 1>{}(p_table, p_index);
      }
    };
  };
};

}; // namespace details

namespace details {

template <typename T, table_memory_layout MemoryLayout> struct __table_iterator;

}; // namespace details

struct no_hooks {
  template <typename TableType, int Col>
  void
  before_remove(TableType &p_table, uimax_t p_index,
                decltype(TableType::template col_type<Col>()) &p_element){};
};

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

template <typename... Types> struct table_col_types;
template <typename Type0> struct table_col_types<Type0> {
  static constexpr ui8_t COL_COUNT = 1;
  template <int N> static auto col_element_type();
  template <> static auto col_element_type<0>() { return Type0{}; };
};

template <typename T, table_memory_layout MemoryLayout> struct col;
template <typename T> struct col<T, table_memory_layout::POOL> {
  T *m_data;
  T *&data() { return m_data; };
  T &at(uimax_t p_index) { return m_data[p_index]; };
};
template <typename T> struct col<T, table_memory_layout::VECTOR> {
  T *m_data;
  T *&data() { return m_data; };
  T &at(uimax_t p_index) { return m_data[p_index]; };
};

template <typename Type0, typename Type1> struct table_col_types<Type0, Type1> {
  static constexpr ui8_t COL_COUNT = 2;
  template <int N> static auto col_element_type();
  template <> static auto col_element_type<0>() { return Type0{}; };
  template <> static auto col_element_type<1>() { return Type1{}; };
};

template <typename ColTypes, table_memory_layout MemoryLayout,
          int Col = ColTypes::COL_COUNT>
struct table_cols;
template <typename ColTypes, table_memory_layout MemoryLayout>
struct table_cols<ColTypes, MemoryLayout, 1> {
  col<decltype(ColTypes::template col_element_type<0>()), MemoryLayout> m_col_0;

  template <int N> auto &col();
  template <> auto &col<0>() { return m_col_0; };
};

template <typename ColTypes, table_memory_layout MemoryLayout>
struct table_cols<ColTypes, MemoryLayout, 2> {
  col<decltype(ColTypes::template col_element_type<0>()), MemoryLayout> m_col_0;
  col<decltype(ColTypes::template col_element_type<1>()), MemoryLayout> m_col_1;

  template <int N> auto &col();
  template <> auto &col<0>() { return m_col_0; };
  template <> auto &col<1>() { return m_col_1; };
};

template <typename ColTypes,
          table_memory_layout MemoryLayout = table_memory_layout::POOL,
          typename Hooks = no_hooks>
struct table {
  using meta_type = details::table_meta<MemoryLayout>;
  static constexpr ui8_t COL_COUNT = ColTypes::COL_COUNT;
  meta_type m_meta;
  Hooks m_hooks;
  table_cols<ColTypes, MemoryLayout> m_cols;

  meta_type &meta() { return m_meta; };

  void register_hooks(const Hooks &p_hooks) { m_hooks = p_hooks; };

  void allocate(uimax_t p_capacity) {
    details::table_allocate<table>(*this, p_capacity);
  };

  void free() { details::table_free<table>(*this); };

  template <typename... Types> uimax_t push_back(const Types &... p_elements) {
    return details::table_push_back<table, Types...>(*this, p_elements...)
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

  ui8_t try_resize(uimax_t p_new_capacity) {
    if (m_capacity < p_new_capacity) {
      auto l_calculated_new_capacity = m_capacity;
      if (l_calculated_new_capacity == 0) {
        l_calculated_new_capacity = 1;
      };
      while (l_calculated_new_capacity < p_new_capacity) {
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
    m_count -= 1;
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

template <int Col>
struct table_remove_at_specialized<Col, table_memory_layout::POOL> {
  template <typename TableType>
  void operator()(TableType &p_table, uimax_t p_index) {
    if constexpr (Col < TableType::COL_COUNT - 1) {
      table_remove_at_specialized<Col + 1, table_memory_layout::POOL>{}(
          p_table, p_index);
    }
  };
};

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

  ui8_t try_resize(uimax_t p_new_capacity) {
    if (m_capacity < p_new_capacity) {
      auto l_calculated_new_capacity = m_capacity;
      if (l_calculated_new_capacity == 0) {
        l_calculated_new_capacity = 1;
      };
      while (l_calculated_new_capacity < p_new_capacity) {
        l_calculated_new_capacity *= 2;
      }
      m_capacity = l_calculated_new_capacity;
      return 1;
    }
    return 0;
  };

  void remove_at(uimax_t p_index) { m_count -= 1; };
};

template <int Col>
struct table_remove_at_specialized<Col, table_memory_layout::VECTOR> {
  template <typename TableType>
  void operator()(TableType &p_table, uimax_t p_index) {
    if (p_index < p_table.meta().m_capacity) {
      auto &l_col = p_table.template col<Col>();
      sys::memmove_up(l_col.data(), p_index + 1, 1, 1);
    }

    if constexpr (Col < TableType::COL_COUNT - 1) {
      table_remove_at_specialized<Col + 1, table_memory_layout::VECTOR>{}(
          p_table, p_index);
    }
  };
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

// TODO
template <> struct table_meta<table_memory_layout::POOL_FIXED> {
  static const table_memory_layout MEMORY_LAYOUT =
      table_memory_layout::POOL_FIXED;

  struct chunk {
    container::vector<uimax_t> m_free_elements;
    container::vector<uimax_t> m_allocated_elements;
  };
};

}; // namespace details

}; // namespace orm