#pragma once

#include <cor/container.hpp>
#include <cor/cor.hpp>
#include <sys/sys.hpp>

namespace orm {

namespace details {

struct table_meta {

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
      p_table.template col<N>() = (element_type *)sys::malloc(
          p_table.m_meta.m_capacity * sizeof(element_type));
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
    p_table.m_meta.free();
    __free_recursive<0>{}(p_table);
  };

private:
  template <int N> struct __free_recursive {
    void operator()(TableType &p_table) {
      using element_type = decltype(p_table.template col_type<N>());
      sys::free(p_table.template col<N>());
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
      auto l_new_byte_size = p_table.m_meta.m_capacity * sizeof(element_type);
      auto *&l_col_ptr = p_table.template col<Col>();
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
    m_index = p_table.m_meta.next_free_element();
    if (m_index == -1) {
      if (p_table.m_meta.try_resize(p_table.m_meta.m_capacity + 1)) {
        table_realloc_cols<TableType>{}(p_table);
        m_index = p_table.m_meta.next_free_element();
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
      p_table.template col<Col>()[p_index] = p_element;
      if constexpr (Col < TableType::COL_COUNT - 1) {
        __table_push_back_col<Col + 1, LocalTypes...>{}(p_table, p_index,
                                                        p_next...);
      }
    };
  };
};

template <typename TableType> struct table_remove_at {
  table_remove_at(TableType &p_table, uimax_t p_index) {
    p_table.m_meta.remove_at(p_index);
    __table_remove_at<0>{}(p_table, p_index);
  };

private:
  template <int Col> struct __table_remove_at {
    void operator()(TableType &p_table, uimax_t p_index) {
      p_table.m_hooks.template before_remove<TableType, Col>(
          p_table, p_index, p_table.template col<Col>()[p_index]);
      if constexpr (Col < TableType::COL_COUNT - 1) {
        __table_remove_at<Col + 1>{}(p_table, p_index);
      }
    };
  };
};

}; // namespace details

template <typename... Types> struct table;
template <typename... Tables> struct db;

template <typename Type0> struct table<Type0> {
  static constexpr ui8_t COL_COUNT = 1;
  details::table_meta m_meta;
  Type0 *m_col0;

  void allocate(uimax_t p_capacity) {
    details::table_allocate<table>(*this, p_capacity);
  };

  void free() { details::table_free<table>(*this); };

  template <int N> auto &col();
  template <> auto &col<0>() { return m_col0; };

  template <int N> static auto col_type();
  template <> static auto col_type<0>() { return Type0{}; };
};

struct no_hooks {
  template <typename TableType, int Col>
  void
  before_remove(TableType &p_table, uimax_t p_index,
                decltype(TableType::template col_type<Col>()) &p_element){};
};

template <typename Type0, typename Type1, typename Hooks>
struct table<Type0, Type1, Hooks> {
  static constexpr ui8_t COL_COUNT = 2;
  details::table_meta m_meta;
  Hooks m_hooks;
  Type0 *m_col0;
  Type1 *m_col1;

  void register_hooks(const Hooks &p_hooks) { m_hooks = p_hooks; };

  void allocate(uimax_t p_capacity) {
    details::table_allocate<table>(*this, p_capacity);
  };

  void free() { details::table_free<table>(*this); };

  uimax_t push_back(const Type0 &p_0, const Type1 &p_1) {
    return details::table_push_back<table, Type0, Type1>(*this, p_0, p_1)
        .m_index;
  };

  void remove_at(uimax_t p_index) {
    details::table_remove_at<table>(*this, p_index);
  };

  template <int N> auto &col();
  template <> auto &col<0>() { return m_col0; };
  template <> auto &col<1>() { return m_col1; };

  template <int N> static auto col_type();
  template <> static auto col_type<0>() { return Type0{}; };
  template <> static auto col_type<1>() { return Type1{}; };

  template <int N> auto range() {
    return container::range_ref<decltype(col_type<N>())>(col<N>(),
                                                         m_meta.m_count);
  };
};

namespace details {};

template <typename Table0> struct db<Table0> {
  static constexpr ui8_t TABLE_COUNT = 1;

  Table0 m_table_0;

  void allocate(uimax_t p_capacity) { m_table_0.allocate(p_capacity); };

  void free() { m_table_0.free(); };

  template <int T> auto &table();
  template <> auto &table<0>() { return m_table_0; };
};

}; // namespace orm