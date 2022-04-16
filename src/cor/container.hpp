#pragma once

#include <cor/cor.hpp>
#include <sys/sys.hpp>

#define BOOST_NO_EXCEPTIONS
#include <boost/container/vector.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/simple_segregated_storage.hpp>
#include <boost/range/algorithm.hpp>

namespace container {

struct malloc_free_functions {
  static void *malloc(uimax_t p_size) { return sys::malloc(p_size); };
  static void free(void *p_ptr) { sys::free(p_ptr); };
  static void *realloc(void *p_ptr, uimax_t p_size) {
    return sys::realloc(p_ptr, p_size);
  };
};

template <typename T> struct range {

  T *m_begin;
  uimax_t m_count;

  void copy_to(const range &p_to) const {
    sys::memcpy(p_to.m_begin, m_begin, m_count * sizeof(T));
  };
};

template <typename T, typename AllocFunctions = malloc_free_functions>
struct span {
  T *m_data;
  uimax_t m_count;

  void allocate(uimax_t p_count) {
    m_data = (T *)AllocFunctions::malloc(p_count * sizeof(T));
    m_count = p_count;
  };

  void free() { AllocFunctions::free(m_data); };

  void realloc(uimax_t p_new_count) {
    m_count = p_new_count;
    m_data = (T *)AllocFunctions::realloc(m_data, m_count * sizeof(T));
  };

  void memmove_down(uimax_t p_break_index, uimax_t p_move_delta,
                    uimax_t p_chunk_count) {
    T *l_src = m_data + p_break_index;
    T *l_dst = l_src + p_move_delta;
    uimax_t l_byte_size = p_chunk_count * sizeof(T);

    block_debug(__assert_memove(l_dst, l_src, p_chunk_count););

    sys::memmove(l_dst, l_src, l_byte_size);
  };

  void memmove_up(uimax_t p_break_index, uimax_t p_move_delta,
                  uimax_t p_chunk_count) {
    T *l_src = m_data + p_break_index;
    T *l_dst = l_src - p_move_delta;
    uimax_t l_byte_size = p_chunk_count * sizeof(T);

    block_debug(__assert_memove(l_dst, l_src, p_chunk_count););

    sys::memmove(l_dst, l_src, l_byte_size);
  };

  range<T> range() {
    container::range<T> l_range;
    l_range.m_begin = m_data;
    l_range.m_count = m_count;
    return l_range;
  };

private:
  void __assert_memove(T *p_dst, T *p_src, uimax_t p_chunk_count) {
    T *l_end_excluded = m_data + m_count + 1;
    sys::sassert(p_src < l_end_excluded);
    sys::sassert(p_dst < l_end_excluded);
    sys::sassert(p_src + p_chunk_count < l_end_excluded);
    sys::sassert(p_dst + p_chunk_count < l_end_excluded);
  };
};

template <typename T, typename AllocFunctions = malloc_free_functions>
struct vector {
  span<T, AllocFunctions> m_span;
  uimax_t m_count;
  uimax_t capacity() const { return m_span.m_count; };

  void allocate(uimax_t p_capacity) {
    m_span.allocate(p_capacity);
    m_count = 0;
  };

  void free() {
    m_span.free();
    m_count = 0;
  };

  uimax_t count() const { return m_count; };

  T &at(uimax_t p_index) {
    assert_debug(p_index < m_count);
    return m_span.m_data[p_index];
  };

  void insert_at(const range<T> &p_range, uimax_t p_index) {
    assert_debug(p_index + p_range.m_count <= capacity());
    uimax_t l_new_count = m_count + p_range.m_count;
    __resize_if_necessary(l_new_count);
    uimax_t l_chunk_count = l_new_count - (p_index + p_range.m_count);
    m_span.memmove_down(p_index, l_chunk_count, p_range.m_count);
    m_count = l_new_count;
    container::range<T> l_target;
    l_target.m_begin = m_span.m_data + p_index;
    l_target.m_count = p_range.m_count;
    p_range.copy_to(l_target);
  };

  void insert_at(const T &p_element, uimax_t p_index) {
    assert_debug(p_index <= capacity());
    uimax_t l_new_count = m_count + 1;
    __resize_if_necessary(l_new_count);
    uimax_t l_chunk_count = m_count - p_index;
    m_span.memmove_down(p_index, l_chunk_count, 1);
    m_count = l_new_count;
    at(p_index) = p_element;
  };

  void push_back(const T &p_element) { insert_at(p_element, m_count); };
  void pop_back() {
    assert_debug(m_count > 0);
    m_count -= 1;
  };

  void clear() { m_count = 0; };

  range<T> range() {
    container::range<T> l_range;
    l_range.m_begin = m_span.m_data;
    l_range.m_count = m_count;
    return l_range;
  };

  struct iterator {
    vector &m_vector;
    uimax_t m_index;

    iterator(vector &p_vector, uimax_t p_index)
        : m_vector(p_vector), m_index(p_index){};

    uimax_t &index() { return m_index; }
    T &operator*() { return m_vector.at(m_index); };
    operator bool() { return m_index < m_vector.m_count; };
    void operator++() { m_index += 1; };
  };

  iterator iter(uimax_t p_begin) { return iterator(*this, p_begin); };

private:
  void __resize_if_necessary(uimax_t p_new_capacity) {
    if (capacity() < p_new_capacity) {
      m_span.realloc(p_new_capacity);
    }
  };
};

template <typename T> struct object_pool_indexed {
private:
  vector<T> data;
  vector<uimax> free_elements;

public:
  object_pool_indexed() = default;
  object_pool_indexed(const object_pool_indexed &) = default;

  object_pool_indexed(uimax p_capacity) { data.reserve(p_capacity); };

public:
  uimax malloc(const T &p_element) {
    if (free_elements.count() > 0) {
      auto l_index = free_elements.at(free_elements.count() - 1);
      free_elements.pop_back();
      data.at(l_index.value) = p_element;
      return l_index;
    } else {
      data.push_back(p_element);
      return uimax(data.count() - 1);
    }
  };

  void free(uimax p_index) {
    sys::sassert(is_element_allocated(p_index)); // TODO add debug for this
    free_elements.push_back(p_index);
  };

  T &at(uimax p_index) {
    sys::sassert(is_element_allocated(p_index)); // TODO add debug for this
    return data.at(p_index.value);
  };

  ui32_t free_elements_size() { return free_elements.count(); };

private:
  i8_t is_element_allocated(uimax p_index) {
    for (auto i = 0; i < free_elements.count(); ++i) {
      auto &l_free_index = free_elements.at(i);
      if (l_free_index == p_index) {
        return 0;
      }
    }

    return 1;
  };
};

}; // namespace container
