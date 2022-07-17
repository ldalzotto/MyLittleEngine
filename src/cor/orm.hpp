#pragma once

#include <cor/container.hpp>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace orm {

template <typename T> struct heap_index_tag { using type = T; };

namespace details {

namespace traits {

template <typename ColType> struct has_heap_index_tag {
  inline static constexpr ui8 value = 0;
};
template <typename T> struct has_heap_index_tag<heap_index_tag<T>> {
  inline static constexpr ui8 value = 1;
};

template <typename T> struct heap_index_col {
  using type = T;
  T *m_data;

  void malloc(uimax p_count) {
    m_data = (T *)sys::malloc(p_count * sizeof(T));
  };

  void free() { sys::free(m_data); };
  void realloc(uimax p_count) {
    m_data = (T *)sys::realloc(m_data, sizeof(T) * p_count);
  };

  T &at(uimax p_index) { return *(m_data + p_index); };
};

template <typename T> struct ptr_col {
  using type = T;
  T *m_data;

  void malloc(uimax p_count) {
    m_data = (T *)sys::malloc(p_count * sizeof(T));
  };

  void free() { sys::free(m_data); };
  void realloc(uimax p_count) {
    m_data = (T *)sys::realloc(m_data, sizeof(T) * p_count);
  };

  T &at(uimax p_index) { return *(m_data + p_index); };
  container::range<T> range(uimax p_begin, uimax p_count) {
    return container::range<T>::make(m_data + p_begin, p_count);
  };

  void memmove_up(uimax p_break_index, uimax p_move_delta,
                  uimax p_chunk_count) {
    sys::memmove_up_t(m_data, p_break_index, p_move_delta, p_chunk_count);
  };
};

template <typename T> struct any_col {

private:
  inline static constexpr auto col_type() {
    if constexpr (traits::has_heap_index_tag<T>::value) {
      return heap_index_col<typename T::type>{};
    } else {
      return ptr_col<T>{};
    }
  };

public:
  using type = decltype(col_type());
};

template <typename T> using any_col_t = typename any_col<T>::type;

template <typename ColType> struct is_heap_index_col {
  inline static constexpr ui8 value = 0;
};

template <typename T> struct is_heap_index_col<heap_index_col<T>> {
  inline static constexpr ui8 value = 1;
};

} // namespace traits

template <typename... Types> struct cols;
template <typename Type0> struct cols<Type0> {
  static constexpr ui8 COL_COUNT = 1;
  traits::any_col_t<Type0> m_col_0;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
};

template <typename Type0, typename Type1> struct cols<Type0, Type1> {
  static constexpr ui8 COL_COUNT = 2;
  traits::any_col_t<Type0> m_col_0;
  traits::any_col_t<Type1> m_col_1;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
  template <> auto &col<1>() { return m_col_1; };
};

template <typename Type0, typename Type1, typename Type2>
struct cols<Type0, Type1, Type2> {
  static constexpr ui8 COL_COUNT = 3;
  traits::any_col_t<Type0> m_col_0;
  traits::any_col_t<Type1> m_col_1;
  traits::any_col_t<Type2> m_col_2;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
  template <> auto &col<1>() { return m_col_1; };
  template <> auto &col<2>() { return m_col_2; };
};

template <typename Type0, typename Type1, typename Type2, typename Type3>
struct cols<Type0, Type1, Type2, Type3> {
  static constexpr ui8 COL_COUNT = 4;
  traits::any_col_t<Type0> m_col_0;
  traits::any_col_t<Type1> m_col_1;
  traits::any_col_t<Type2> m_col_2;
  traits::any_col_t<Type3> m_col_3;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
  template <> auto &col<1>() { return m_col_1; };
  template <> auto &col<2>() { return m_col_2; };
  template <> auto &col<3>() { return m_col_3; };
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

  T *map_to_ptr(const container::heap_paged_chunk &p_chunk) {
    return &(m_data[p_chunk.m_page_index])[p_chunk.m_chunk.m_begin];
  };
};

template <typename... Types> struct heap_paged_cols;
template <typename Type0> struct heap_paged_cols<Type0> {
  static constexpr ui8 COL_COUNT = 1;
  heap_paged_col<Type0> m_col_0;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
};

template <typename Type0, typename Type1> struct heap_paged_cols<Type0, Type1> {
  static constexpr ui8 COL_COUNT = 2;
  heap_paged_col<Type0> m_col_0;
  heap_paged_col<Type1> m_col_1;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
  template <> auto &col<1>() { return m_col_1; };
};

template <typename Type0, typename Type1, typename Type2>
struct heap_paged_cols<Type0, Type1, Type2> {
  static constexpr ui8 COL_COUNT = 3;
  heap_paged_col<Type0> m_col_0;
  heap_paged_col<Type1> m_col_1;
  heap_paged_col<Type2> m_col_2;
  template <ui8 Col> auto &col();
  template <> auto &col<0>() { return m_col_0; };
  template <> auto &col<1>() { return m_col_1; };
  template <> auto &col<2>() { return m_col_2; };
};

template <typename Type0, typename Type1, typename Type2, typename Type3>
struct heap_paged_cols<Type0, Type1, Type2, Type3> {
  static constexpr ui8 COL_COUNT = 4;
  heap_paged_col<Type0> m_col_0;
  heap_paged_col<Type1> m_col_1;
  heap_paged_col<Type2> m_col_2;
  heap_paged_col<Type3> m_col_3;
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
    __at<0, Input...>{}(*this, p_index, p_input...);
  };

  template <typename... Input>
  void set(uimax p_index, const Input &... p_input) {
    assert_debug(p_index < count());
    __set<0, Input...>{}(*this, p_index, p_input...);
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
          l_col.malloc(p_count);
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
          l_col.free();
          free_col<Col + 1>{}(thiz);
        }
      };
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input> struct __at {
    void operator()(table_span_v2 &thiz, uimax p_index, InputFirst p_first,
                    Input... p_input) {
      if constexpr (!::traits::is_none<InputFirst>::value) {
        auto &l_col = thiz.cols().template col<Col>();
        *p_first = &l_col.at(p_index);
      }
      if constexpr (sizeof...(Input) > 0) {
        __at<Col + 1, Input...>{}(thiz, p_index, p_input...);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input> struct __set {
    void operator()(table_span_v2 &thiz, uimax p_index,
                    const InputFirst &p_first, const Input &... p_input) {
      if constexpr (!::traits::is_none<InputFirst>::value) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.at(p_index) = p_first;
      }

      if constexpr (sizeof...(Input) > 0) {
        __set<Col + 1, Input...>{}(thiz, p_index, p_input...);
      }
    };
  };

  template <ui8... Cols, typename... Input>
  void __set_v2(uimax p_index, const Input &... p_input) {
    if constexpr (sizeof...(Cols) > 0) {
    }
  };

  template <ui8 Col, typename InputFirst, typename... Input> struct __range {
    void operator()(table_span_v2 &thiz, InputFirst p_first, Input... p_input) {
      if constexpr (!::traits::is_none<InputFirst>::value) {
        auto &l_col = thiz.cols().template col<Col>();
        *p_first = l_col.range(0, thiz.count());
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
          l_col.realloc(p_new_count);
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
  ui8 has_allocated_elements() { return count() != 0; };

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
      table_vector_memmove_up<0>{}(*this, p_index + 1, 1,
                                   m_meta.m_count - (p_index + 1));
    }

    m_meta.m_count -= 1;
  };

  template <typename... Input> void at(uimax p_index, Input &&... p_input) {
    assert_debug(p_index < count());
    __at<0, Input...>{}(*this, p_index, p_input...);
  };

private:
  template <ui8 Col> struct table_vector_allocate {
    void operator()(table_vector_v2 &thiz, uimax p_capacity) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.malloc(p_capacity);
        table_vector_allocate<Col + 1>{}(thiz, p_capacity);
      }
    };
  };

  template <ui8 Col> struct table_vector_free {
    void operator()(table_vector_v2 &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.free();
        table_vector_free<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col> struct __realloc {
    void operator()(table_vector_v2 &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.realloc(thiz.m_meta.m_capacity);
        __realloc<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input> struct __at {
    void operator()(table_vector_v2 &thiz, uimax p_index, InputFirst p_first,
                    Input... p_input) {
      if constexpr (!::traits::is_none<InputFirst>::value) {
        auto &l_col = thiz.cols().template col<Col>();
        *p_first = &(l_col.at(p_index));
      }
      if constexpr (sizeof...(Input) > 0) {
        __at<Col + 1, Input...>{}(thiz, p_index, p_input...);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input>
  struct table_vector_set_value {

    void operator()(table_vector_v2 &thiz, const InputFirst &p_first,
                    const Input &... p_input) {
      if constexpr (!::traits::is_none<InputFirst>::value) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.at(thiz.m_meta.m_count - 1) = p_first;
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
        l_col.memmove_up(p_break_index, p_move_delta, p_chunk_count);
        table_vector_memmove_up<Col + 1>{}(thiz, p_break_index, p_move_delta,
                                           p_chunk_count);
      }
    };
  };
};

template <typename... Types> struct table_pool_v2 {
  container::pool_intrusive m_meta;
  details::cols<Types...> m_cols;
  static constexpr ui8 COL_COUNT = details::cols<Types...>::COL_COUNT;

  details::cols<Types...> &cols() { return m_cols; };

  void allocate(uimax p_capacity) {
    m_meta.allocate(p_capacity);
    table_pool_allocate<0>{}(*this, p_capacity);
  };

  void free() {
    m_meta.free();
    table_pool_free<0>{}(*this);
  };

  template <typename... Input> uimax push_back(const Input &... p_input) {
    uimax l_index;
    if (m_meta.find_next_realloc(&l_index)) {
      __realloc<0>{}(*this);
    }
    table_pool_set_value<0, const Input &...>{}(*this, l_index, p_input...);
    return l_index;
  };

  template <typename... Input> void at(uimax p_index, Input &&... p_input) {
    assert_debug(p_index < m_meta.m_count);
    __at<0, Input...>{}(*this, p_index, p_input...);
  };

  void remove_at(uimax p_index) { m_meta.free_element(p_index); };
  ui8 has_allocated_elements() { return m_meta.has_allocated_elements(); };

private:
  template <ui8 Col> struct table_pool_allocate {
    void operator()(table_pool_v2 &thiz, uimax p_capacity) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.malloc(p_capacity);
        table_pool_allocate<Col + 1>{}(thiz, p_capacity);
      }
    };
  };

  template <ui8 Col> struct table_pool_free {
    void operator()(table_pool_v2 &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.free();
        table_pool_free<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col> struct __realloc {
    void operator()(table_pool_v2 &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.realloc(thiz.m_meta.m_capacity);
        __realloc<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input>
  struct table_pool_set_value {

    void operator()(table_pool_v2 &thiz, uimax p_index,
                    const InputFirst &p_first, const Input &... p_input) {

      if constexpr (!::traits::is_none<InputFirst>::value) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.at(p_index) = p_first;
      }

      if constexpr (Col + 1 < COL_COUNT) {
        table_pool_set_value<Col + 1, const Input &...>{}(thiz, p_index,
                                                          p_input...);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input> struct __at {
    void operator()(table_pool_v2 &thiz, uimax p_index, InputFirst p_first,
                    Input... p_input) {
      if constexpr (!::traits::is_none<InputFirst>::value) {
        auto &l_col = thiz.cols().template col<Col>();
        *p_first = &(l_col.at(p_index));
      }
      if constexpr (sizeof...(Input) > 0) {
        __at<Col + 1, Input...>{}(thiz, p_index, p_input...);
      }
    };
  };
};

template <typename... Types> struct table_heap_paged_v2 {
  container::heap_paged_intrusive m_meta;
  details::heap_paged_cols<Types...> m_cols;
  static constexpr ui8 COL_COUNT = details::cols<Types...>::COL_COUNT;

  details::heap_paged_cols<Types...> &cols() { return m_cols; };

  void allocate(uimax p_capacity) {
    m_meta.allocate(p_capacity);
    table_heap_paged_allocate<0>{}(*this);
  };

  void free() {
    m_meta.free();
    table_heap_paged_free<0>{}(*this);
  };

  ui8 has_allocated_elements() {
    return m_meta.m_allocated_chunks.m_intrusive.has_allocated_elements();
  };

  template <typename... Input> uimax at(uimax p_index, Input &&... p_input) {
    assert_debug(p_index < m_meta.m_allocated_chunks.count());
    __at<0, Input...>{}(*this, p_index, p_input...);
    return m_meta.m_allocated_chunks.at(p_index).m_chunk.m_size;
  };

  uimax push_back(uimax p_size) {
    uimax l_page_index, l_chunk_index;
    m_meta.find_next_chunk(p_size, &l_page_index, &l_chunk_index);
    if (m_meta.m_state ==
        container::heap_paged_intrusive::state::NewPagePushed) {
      m_meta.clear_state();
      table_heap_paged_push_new_page<0>{}(*this, l_page_index);
    }

    return m_meta.push_found_chunk(p_size, l_page_index, l_chunk_index);
  };

  uimax push_back(uimax p_size, uimax p_alignment) {
    uimax l_page_index, l_chunk_index;
    m_meta.find_next_chunk(p_size, p_alignment, &l_page_index, &l_chunk_index);
    if (m_meta.m_state ==
        container::heap_paged_intrusive::state::NewPagePushed) {
      m_meta.clear_state();
      table_heap_paged_push_new_page<0>{}(*this, l_page_index);
    }

    return m_meta.push_found_chunk(p_size, l_page_index, l_chunk_index);
  };

  void remove_at(uimax p_chunk_index) { m_meta.remove_chunk(p_chunk_index); };

private:
  template <ui8 Col> struct table_heap_paged_allocate {
    void operator()(table_heap_paged_v2 &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.allocate();
        table_heap_paged_allocate<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col> struct table_heap_paged_free {
    void operator()(table_heap_paged_v2 &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.free(thiz.m_meta);
        table_heap_paged_free<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input> struct __at {
    void operator()(table_heap_paged_v2 &thiz, uimax p_index,
                    InputFirst p_first, Input... p_input) {
      if constexpr (!::traits::is_none<InputFirst>::value) {
        using T = typename ::traits::remove_ptr_ref<
            typename ::traits::remove_ptr_ref<InputFirst>::type>::type;
        details::heap_paged_col<T> &l_col = thiz.cols().template col<Col>();
        *p_first = l_col.map_to_ptr(thiz.m_meta.m_allocated_chunks.at(p_index));
      }
      if constexpr (sizeof...(Input) > 0) {
        __at<Col + 1, Input...>{}(thiz, p_index, p_input...);
      }
    };
  };

  template <ui8 Col> struct table_heap_paged_push_new_page {
    void operator()(table_heap_paged_v2 &thiz, uimax p_page_index) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.realloc(thiz.m_meta);
        l_col.allocate_page(thiz.m_meta, p_page_index);
        table_heap_paged_push_new_page<Col + 1>{}(thiz, p_page_index);
      }
    };
  };
};

template <typename... Types> struct table_heap_stacked {
  container::heap_stacked_intrusive m_meta;
  details::cols<Types...> m_cols;
  static constexpr ui8 COL_COUNT = details::cols<Types...>::COL_COUNT;

  details::cols<Types...> &cols() { return m_cols; };

  void allocate(uimax p_capacity) {
    m_meta.allocate(p_capacity);
    table_heap_stacked_allocate<0>{}(*this, p_capacity);
  };

  void free() {
    m_meta.free();
    table_heap_stacked_free<0>{}(*this);
  };

  uimax count() { return m_meta.count(); };

  void push_back(uimax p_size, uimax p_alignment) {
    uimax l_chunk_index = -1;
    if (!m_meta.find_next_chunk(p_size, p_alignment, &l_chunk_index)) {
      m_meta.increase_capacity(p_size);
      table_heap_stacked_realloc<0>{}(*this);
      m_meta.find_next_chunk(p_size, p_alignment, &l_chunk_index);
    }
    assert_debug(l_chunk_index != -1);
  };

  void push_back_no_realloc(uimax p_size, uimax p_alignment) {
    uimax l_chunk_index = -1;
    m_meta.find_next_chunk(p_size, p_alignment, &l_chunk_index);
    assert_debug(l_chunk_index != -1);
  };

  template <typename... Input> void at(uimax p_index, Input... out_ranges) {
    const container::heap_chunk &l_chunk =
        m_meta.m_allocated_chunks.at(p_index);
    table_heap_stacked_map_to_range<0, Input...>{}(*this, p_index, l_chunk,
                                                   out_ranges...);
  };

private:
  template <ui8 Col> struct table_heap_stacked_allocate {
    void operator()(table_heap_stacked &thiz, uimax p_capacity) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.malloc(p_capacity);
        table_heap_stacked_allocate<Col + 1>{}(thiz, p_capacity);
      }
    };
  };

  template <ui8 Col> struct table_heap_stacked_free {
    void operator()(table_heap_stacked &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.free();
        table_heap_stacked_free<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col> struct table_heap_stacked_realloc {
    void operator()(table_heap_stacked &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        l_col.realloc(thiz.m_meta.m_capacity);
        table_heap_stacked_realloc<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col, typename FirstRange, typename... Input>
  struct table_heap_stacked_map_to_range {
    void operator()(table_heap_stacked &thiz, uimax p_index,
                    const container::heap_chunk &p_chunk, FirstRange out_range,
                    Input... out_ranges) {
      if constexpr (!::traits::is_none<FirstRange>::value) {
        auto &l_col = thiz.cols().template col<Col>();
        using col_t = typename ::traits::remove_ptr_ref<decltype(l_col)>::type;
        if constexpr (details::traits::is_heap_index_col<col_t>::value) {
          *out_range = &l_col.at(p_index);
        } else {
          *out_range = l_col.range(p_chunk.m_begin, p_chunk.m_size);
        }
      }
      if constexpr (sizeof...(Input) > 0) {
        table_heap_stacked_map_to_range<Col + 1, Input...>{}(
            thiz, p_index, p_chunk, out_ranges...);
      }
    };
  };
};

template <typename Key, typename... Types> struct table_hashmap {
  container::hashmap_intrusive<Key> m_meta;
  details::cols<Types...> m_cols;
  static constexpr ui8 COL_COUNT = details::cols<Types...>::COL_COUNT;

  details::cols<Types...> &cols() { return m_cols; };

  void allocate() {
    m_meta.allocate();
    table_hashmap_allocate<0>{}(*this);
  };

  void free() {
    m_meta.free();
    table_hashmap_free<0>{}(*this);
  };

  template <typename... Input>
  uimax push_back(const Key &p_key, const Input &... p_input) {
    uimax l_index;
    if (m_meta.push_back_realloc(p_key, &l_index)) {
      __realloc<0>{}(*this);
    }
    table_hashmap_set_value<0, const Input &...>{}(*this, l_index, p_input...);
    return l_index;
  };

  template <typename... Input> void at(const Key &p_key, Input &&... p_input) {
    uimax l_key_index = m_meta.find_key_index(p_key);
    assert_debug(l_key_index != -1);
    __at<0, Input...>{}(*this, l_key_index, p_input...);
  };

  void remove_at(uimax p_index) { m_meta.free_element(p_index); };
  ui8 has_allocated_elements() { return m_meta.has_allocated_elements(); };

private:
  template <ui8 Col> struct table_hashmap_allocate {
    void operator()(table_hashmap &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        using T = typename ::traits::remove_ptr_ref<decltype(l_col)>::type;
        l_col = (T *)sys::malloc(0);
        table_hashmap_allocate<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col> struct table_hashmap_free {
    void operator()(table_hashmap &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        sys::free(l_col);
        table_hashmap_free<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col> struct __realloc {
    void operator()(table_hashmap &thiz) {
      if constexpr (Col < COL_COUNT) {
        auto &l_col = thiz.cols().template col<Col>();
        using T = typename ::traits::remove_ptr_ref<decltype(l_col)>::type;
        l_col = (T *)sys::realloc(
            l_col, thiz.m_meta.m_keys_intrisic.m_capacity * sizeof(T));
        __realloc<Col + 1>{}(thiz);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input>
  struct table_hashmap_set_value {

    void operator()(table_hashmap &thiz, uimax p_index,
                    const InputFirst &p_first, const Input &... p_input) {

      if constexpr (!::traits::is_none<InputFirst>::value) {
        thiz.cols().template col<Col>()[thiz.m_meta.m_count - 1] = p_first;
      }

      if constexpr (Col + 1 < COL_COUNT) {
        table_hashmap_set_value<Col + 1, const Input &...>{}(thiz, p_index,
                                                             p_input...);
      }
    };
  };

  template <ui8 Col, typename InputFirst, typename... Input> struct __at {
    void operator()(table_hashmap &thiz, const uimax &p_index,
                    InputFirst p_first, Input... p_input) {
      if constexpr (!::traits::is_none<InputFirst>::value) {
        *p_first = &(thiz.cols().template col<Col>())[p_index];
      }
      if constexpr (sizeof...(Input) > 0) {
        __at<Col + 1, Input...>{}(thiz, p_index, p_input...);
      }
    };
  };
};

}; // namespace orm
