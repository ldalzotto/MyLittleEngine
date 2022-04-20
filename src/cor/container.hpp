#pragma once

#include <cor/algorithm.hpp>
#include <cor/cor.hpp>
#include <cor/hash.hpp>
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

  using element_type = T;

  T *m_begin;
  uimax_t m_count;

  static range make(T *p_begin, uimax_t p_count) {
    range l_range;
    l_range.m_begin = p_begin;
    l_range.m_count = p_count;
    return l_range;
  };

  const uimax_t &count() const { return m_count; };

  T &at(uimax_t p_index) {
    assert_debug(p_index < m_count);
    return m_begin[p_index];
  };

  const T &at(uimax_t p_index) const { return ((range *)this)->at(p_index); };

  void copy_to(const range &p_to) const {
    assert_debug(m_count <= p_to.m_count);
    sys::memcpy(p_to.m_begin, m_begin, m_count * sizeof(T));
  };

  void copy_from(const range &p_from) const {
    assert_debug(p_from.m_count <= m_count);
    sys::memcpy(m_begin, p_from.m_begin, m_count * sizeof(T));
  };

  uimax_t size_bytes() const { return sizeof(T) * m_count; };
};

template <typename T> struct range_ref {
  T *&m_begin;
  uimax_t &m_count;

  range_ref(T *&p_begin, uimax_t &p_count)
      : m_begin(p_begin), m_count(p_count){

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

  uimax_t &count() { return m_count; };

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

  T &at(uimax_t p_index) {
    assert_debug(p_index < m_count);
    return m_data[p_index];
  }

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

  const T &at(uimax_t p_index) const { return ((vector *)this)->at(p_index); };

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

  void remove_at(uimax_t p_index) {
    assert_debug(p_index < m_count);
    m_span.memmove_up(p_index, 1, 1);
    m_count -= 1;
  };

  void clear() { m_count = 0; };

  range<T> range() {
    container::range<T> l_range;
    l_range.m_begin = m_span.m_data;
    l_range.m_count = m_count;
    return l_range;
  };

private:
  void __resize_if_necessary(uimax_t p_new_capacity) {
    if (capacity() < p_new_capacity) {
      auto l_calculated_new_capacity = capacity();
      if (l_calculated_new_capacity == 0) {
        l_calculated_new_capacity = 1;
      };
      while (l_calculated_new_capacity < p_new_capacity) {
        l_calculated_new_capacity *= 2;
      }

      m_span.realloc(l_calculated_new_capacity);
    }
  };
};

template <typename T> struct object_pool_indexed {
private:
  vector<T> m_data;
  vector<uimax_t> m_free_elements;

public:
  void allocate(uimax_t p_capacity) {
    m_data.allocate(p_capacity);
    m_free_elements.allocate(0);
  };

  void free() {
    m_data.free();
    m_free_elements.free();
  };

public:
  uimax_t malloc(const T &p_element) {
    if (has_free_elements()) {
      auto l_index = m_free_elements.at(m_free_elements.count() - 1);
      m_free_elements.pop_back();
      m_data.at(l_index) = p_element;
      return l_index;
    } else {
      m_data.push_back(p_element);
      return m_data.count() - 1;
    }
  };

  void free(uimax_t p_index) {
    assert_debug(__is_element_allocated(p_index));
    m_free_elements.push_back(p_index);
  };

  T &at(uimax_t p_index) {
    assert_debug(__is_element_allocated(p_index));
    return m_data.at(p_index);
  };

  ui8_t is_element_allocated(uimax_t p_index) const {
    return __is_element_allocated(p_index);
  };

  ui8_t has_free_elements() const { return m_free_elements.count() > 0; };

private:
  ui8_t __is_element_allocated(uimax_t p_index) {
    for (auto i = 0; i < m_free_elements.count(); ++i) {
      auto &l_free_index = m_free_elements.at(i);
      if (l_free_index == p_index) {
        return 0;
      }
    }

    return 1;
  };
};

struct heap_chunk {
  uimax_t m_begin;
  uimax_t m_size;
};

namespace heap_chunks {
static inline ui8_t find_next_block(const range<heap_chunk> &p_chunks,
                                    uimax_t p_size, uimax_t *out_chunk_index) {
  assert_debug(p_size > 0);
  for (auto l_chunk_it = 0; l_chunk_it < p_chunks.count(); ++l_chunk_it) {
    const heap_chunk &l_chunk = p_chunks.at(l_chunk_it);
    if (l_chunk.m_size >= p_size) {
      *out_chunk_index = l_chunk_it;
      return 1;
    }
  }
  return 0;
};

static inline void defragment(vector<heap_chunk> &p_chunks) {
  if (p_chunks.count() > 0) {
    auto l_range = p_chunks.range();
    algorithm::sort(l_range, [&](heap_chunk &p_left, heap_chunk &p_right) {
      return p_left.m_begin < p_right.m_begin;
    });

    for (auto l_range_reverse = p_chunks.count() - 1; l_range_reverse >= 1;
         l_range_reverse--) {
      auto &l_next = p_chunks.at(l_range_reverse);
      auto &l_previous = p_chunks.at(l_range_reverse - 1);
      if (l_previous.m_begin + l_previous.m_size == l_next.m_begin) {
        l_previous.m_size += l_next.m_size;
        p_chunks.pop_back();
        l_range_reverse -= 1;
      }
    }
  }
};

}; // namespace heap_chunks

struct heap_intrusive {
  uimax_t m_chunk_count;
  uimax_t m_current_count;
  vector<heap_chunk> m_free_chunks;
  object_pool_indexed<heap_chunk> m_allocated_chunk;

  enum class state { UNDEFINED = 0, NEW_CHUNK_PUSHED = 1 } m_state;

  uimax_t &count() { return m_current_count; };

  void allocate(uimax_t p_chunk_size) {
    m_chunk_count = p_chunk_size;
    m_free_chunks.allocate(0);
    m_allocated_chunk.allocate(0);
    m_current_count = 0;
    m_state = state::UNDEFINED;
  };

  void free() {
    m_free_chunks.free();
    m_allocated_chunk.free();
  };

  void clear_state() { m_state = state::UNDEFINED; };

  uimax_t find_next_chunk(uimax_t p_size) {
    uimax_t l_chunk_index = -1;
    if (!heap_chunks::find_next_block(m_free_chunks.range(), p_size,
                                      &l_chunk_index)) {
      heap_chunks::defragment(m_free_chunks);
      if (!heap_chunks::find_next_block(m_free_chunks.range(), p_size,
                                        &l_chunk_index)) {
        __push_new_chunk();
        m_state = state::NEW_CHUNK_PUSHED;
        heap_chunks::find_next_block(m_free_chunks.range(), p_size,
                                     &l_chunk_index);
      }
    }
    assert_debug(l_chunk_index != -1);
    return l_chunk_index;
  };

  uimax_t push_found_chunk(uimax_t p_size, uimax_t p_free_chunk_indexs) {
    heap_chunk &l_free_chunk = m_free_chunks.at(p_free_chunk_indexs);
    heap_chunk l_chunk;
    l_chunk.m_begin = l_free_chunk.m_begin;
    l_chunk.m_size = p_size;
    uimax_t l_allocated_chunk_index = m_allocated_chunk.malloc(l_chunk);
    if (l_free_chunk.m_size > p_size) {
      l_free_chunk.m_size -= p_size;
      l_free_chunk.m_begin += p_size;
    } else {
      m_free_chunks.remove_at(p_free_chunk_indexs);
    }
    return l_allocated_chunk_index;
  };

  void free(uimax_t p_index) {
    auto &l_chunk = m_allocated_chunk.at(p_index);
    m_free_chunks.push_back(l_chunk);
    m_allocated_chunk.free(p_index);
  };

private:
  void __push_new_chunk() {
    heap_chunk l_chunk;
    l_chunk.m_begin = m_current_count;
    l_chunk.m_size = m_chunk_count;
    m_free_chunks.push_back(l_chunk);
    m_current_count += l_chunk.m_size;
  };
};

struct heap {
  heap_intrusive m_intrusive;
  span<ui8_t> m_buffer;

  void allocate(uimax_t p_chunk_size) {
    m_intrusive.allocate(p_chunk_size);
    m_buffer.allocate(0);
  };

  void free() {
    m_buffer.free();
    m_intrusive.free();
  };

  uimax_t malloc(uimax_t p_size) {
    uimax_t l_chunk_index = m_intrusive.find_next_chunk(p_size);
    if (m_intrusive.m_state == heap_intrusive::state::NEW_CHUNK_PUSHED) {
      __push_new_chunk();
      m_intrusive.clear_state();
    }
    m_intrusive.push_found_chunk(p_size, l_chunk_index);
  };

  ui8_t *at(uimax_t p_index) {
    return m_buffer.m_data + m_intrusive.m_allocated_chunk.at(p_index).m_begin;
  };

  void free(uimax_t p_index) { m_intrusive.free(p_index); };

private:
  void __push_new_chunk() {
    m_buffer.realloc(m_buffer.count() + m_intrusive.m_chunk_count);
  };
};

// TODO
template <typename Key, typename Mapped, typename Hash = hash::hash<Key>>
struct unordered_map {
private:
  static const uimax_t s_invalid_hash = -1;
  vector<Key> m_keys;
  vector<Mapped> m_values;
  vector<ui8_t> m_has_value;
  uimax_t m_scale_factor;

public:
  void allocate(uimax_t p_capacity) {
    m_keys.allocate(p_capacity);
    m_values.allocate(p_capacity);
    m_has_value.allocate(p_capacity);
    m_scale_factor = 1;
  };

  void free() {
    m_keys.free();
    m_values.free();
    m_has_value.free();
  };

private:
};

}; // namespace container

// hash specifications
namespace hash {
template <typename T> struct hash<container::range<T>> {
  hash() = delete;
  uimax_t operator()(const container::range<T> &p_range) {
    return hash_function(p_range.m_begin, p_range.size_bytes());
  };
};
}; // namespace hash