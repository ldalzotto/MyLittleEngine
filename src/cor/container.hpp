#pragma once

#include <cor/algorithm.hpp>
#include <cor/allocations.hpp>
#include <cor/assertions.hpp>
#include <cor/types.hpp>
#include <sys/sys.hpp>

namespace container {

template <typename T> struct range {

  using element_type = T;

  T *m_begin;
  uimax m_count;

  static range make(T *p_begin, uimax p_count) {
    range l_range;
    l_range.m_begin = p_begin;
    l_range.m_count = p_count;
    return l_range;
  };

  const T *data() const { return (const T*)m_begin; };
  const uimax &count() const { return m_count; };
  uimax size_of() const { return sizeof(T) * m_count; };

  T &at(uimax p_index) {
    assert_debug(p_index < m_count);
    return m_begin[p_index];
  };

  const T &at(uimax p_index) const { return ((range *)this)->at(p_index); };

  void copy_to(const range &p_to) const {
    assert_debug(m_count <= p_to.m_count);
    sys::memcpy(p_to.m_begin, m_begin, m_count * sizeof(T));
  };

  void copy_from(const range &p_from) const {
    assert_debug(p_from.m_count <= m_count);
    sys::memcpy(m_begin, p_from.m_begin, m_count * sizeof(T));
  };

  template <typename TT> void copy_from(const TT &p_value) {
    assert_debug(sizeof(TT) <= m_count);
    sys::memcpy(m_begin, (void *)&p_value, sizeof(TT));
  };

  void zero() { sys::memset(m_begin, 0, m_count * sizeof(T)); };

  range<T> slide(uimax p_count) {
    assert_debug(m_count >= p_count);
    return range<T>::make(m_begin + p_count, m_count - p_count);
  };

  void slide_self(uimax p_count) {
    assert_debug(m_count >= p_count);
    m_begin += p_count;
    m_count -= p_count;
  };

  template <typename TT> range &stream(const TT &p_value) {
    copy_from(p_value);
    slide_self(sizeof(p_value));
    return *this;
  };

  template <typename TT> range<TT> cast_to() {
    range<TT> l_return;
    l_return.m_begin = (TT *)m_begin;
    l_return.m_count = (m_count * sizeof(T)) / sizeof(TT);
    return l_return;
  };

  template <typename TT> range<TT> cast_to() const {
    return ((range *)this)->cast_to<TT>();
  };

  ui8 is_contained_by(const range<T> &p_other) const {
    return sys::memcmp(m_begin, p_other.m_begin, size_of()) == 0;
  };
};

template <typename T, int N> struct arr {
  T m_data[N];
  container::range<T> range() { return container::range<T>::make(m_data, N); };
  T *data() { return m_data; };
  const T *data() const { return m_data; };
};

template <typename T, typename AllocFunctions = malloc_free_functions>
struct span {
  T *m_data;
  uimax m_count;

  void allocate(uimax p_count) {
    m_data = (T *)AllocFunctions::malloc(p_count * sizeof(T));
    m_count = p_count;
  };

  void free() { AllocFunctions::free(m_data); };

  uimax &count() { return m_count; };
  T *&data() { return m_data; };

  uimax size_of() const { return m_count * sizeof(T); };

  void realloc(uimax p_new_count) {
    m_count = p_new_count;
    m_data = (T *)AllocFunctions::realloc(m_data, m_count * sizeof(T));
  };

  void resize(uimax p_new_count) {
    if (p_new_count > m_count) {
      realloc(p_new_count);
    }
  };

  void memmove_down(uimax p_break_index, uimax p_move_delta,
                    uimax p_chunk_count) {
    T *l_src = m_data + p_break_index;
    T *l_dst = l_src + p_move_delta;
    uimax l_byte_size = p_chunk_count * sizeof(T);

    block_debug([&]() { __assert_memove(l_dst, l_src, p_chunk_count); });

    sys::memmove(l_dst, l_src, l_byte_size);
  };

  void memmove_up(uimax p_break_index, uimax p_move_delta,
                  uimax p_chunk_count) {
    T *l_src = m_data + p_break_index;
    T *l_dst = l_src - p_move_delta;
    uimax l_byte_size = p_chunk_count * sizeof(T);

    block_debug([&]() { __assert_memove(l_dst, l_src, p_chunk_count); });

    sys::memmove(l_dst, l_src, l_byte_size);
  };

  T &at(uimax p_index) {
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
  void __assert_memove(T *p_dst, T *p_src, uimax p_chunk_count) {
    T *l_end_excluded = m_data + m_count + 1;
    sys::sassert(p_src < l_end_excluded);
    sys::sassert(p_dst < l_end_excluded);
    sys::sassert(p_src + p_chunk_count < l_end_excluded);
    sys::sassert(p_dst + p_chunk_count < l_end_excluded);
  };
};

struct vector_intrusive {
  uimax m_count;
  uimax m_capacity;

  void allocate(uimax p_capacity) {
    m_capacity = p_capacity;
    m_count = 0;
  };

  i8 find_next_realloc(uimax *out_index) {
    i8 l_realloc = 0;
    if (m_count == m_capacity) {
      __realloc(1);
      l_realloc = 1;
    }
    m_count += 1;
    *out_index = m_count;
    return l_realloc;
  };

  i8 add_realloc(uimax p_delta_count) {
    i8 l_realloc = 0;
    if (m_count == m_capacity) {
      __realloc(p_delta_count);
      l_realloc = 1;
    }
    m_count += p_delta_count;
    return l_realloc;
  };

private:
  void __realloc(uimax p_delta_count) {
    auto l_new_capacity = m_capacity + p_delta_count;
    auto l_calculated_new_capacity = m_capacity;
    if (l_calculated_new_capacity == 0) {
      l_calculated_new_capacity = 1;
    };
    while (l_calculated_new_capacity < l_new_capacity) {
      l_calculated_new_capacity *= 2;
    }
    m_capacity = l_calculated_new_capacity;
  };
};

template <typename T, typename AllocFunctions = malloc_free_functions>
struct vector {

  vector_intrusive m_intrusive;
  T *m_data;

  uimax &capacity() { return m_intrusive.m_capacity; };
  uimax &count() { return m_intrusive.m_count; };

  void allocate(uimax p_capacity) {
    m_intrusive.allocate(p_capacity);
    m_data = (T *)sys::malloc(sizeof(T) * m_intrusive.m_capacity);
  };

  void free() { sys::free(m_data); };

  T &at(uimax p_index) {
    assert_debug(p_index < count());
    return m_data[p_index];
  };

  const T &at(uimax p_index) const { return ((vector *)this)->at(p_index); };

  void insert_at(const T &p_element, uimax p_index) {
    assert_debug(p_index <= capacity());
    if (m_intrusive.add_realloc(1)) {
      __realloc();
    }
    uimax l_chunk_count = count() - 1 - p_index;
    sys::memmove_down_t(m_data, p_index, l_chunk_count, 1);
    at(p_index) = p_element;
  };

  void push_back(const T &p_element) {
    if (m_intrusive.add_realloc(1)) {
      __realloc();
    }
    at(m_intrusive.m_count - 1) = p_element;
  };

  void pop_back() {
    assert_debug(count() > 0);
    m_intrusive.m_count -= 1;
  };

  void remove_at(uimax p_index) {
    assert_debug(p_index < count() && count() > 0);
    if (p_index < count() - 1) {
      sys::memmove_up_t(m_data, p_index + 1, 1, 1);
    }

    m_intrusive.m_count -= 1;
  };

  void clear() { m_intrusive.m_count = 0; };

  range<T> range() {
    container::range<T> l_range;
    l_range.m_begin = m_data;
    l_range.m_count = m_intrusive.m_count;
    return l_range;
  };

private:
  void __realloc() {
    m_data = (T *)sys::realloc(m_data, m_intrusive.m_capacity * sizeof(T));
  };
};

struct pool_intrusive {
  uimax m_count;
  uimax m_capacity;
  vector<uimax> m_free_elements;

  void allocate(uimax p_capacity) {
    m_count = 0;
    m_capacity = p_capacity;
    m_free_elements.allocate(0);
  };

  void free() { m_free_elements.free(); };

  i8 find_next_realloc(uimax *out_index) {
    if (m_free_elements.count() > 0) {
      uimax l_free_index = m_free_elements.at(m_free_elements.count() - 1);
      m_free_elements.pop_back();
      *out_index = l_free_index;
      return 0;
    } else {
      if (m_count == m_capacity) {
        __re_capacitate(1);
      }
      m_count += 1;
      *out_index = m_count - 1;
      return 1;
    }
  };

  void free_element(uimax p_index) {
    assert_debug(is_element_allocated(p_index));
    m_free_elements.push_back(p_index);
  };

  i8 is_element_allocated(uimax p_index) {
    for (auto i = 0; i < m_free_elements.count(); ++i) {
      if (m_free_elements.at(i) == p_index) {
        return 0;
      }
    }
    return 1;
  };

  bool has_allocated_elements() { return m_free_elements.count() != m_count; };

private:
  void __re_capacitate(uimax p_delta_count) {
    auto l_new_capacity = m_capacity + p_delta_count;
    auto l_calculated_new_capacity = m_capacity;
    if (l_calculated_new_capacity == 0) {
      l_calculated_new_capacity = 1;
    };
    while (l_calculated_new_capacity < l_new_capacity) {
      l_calculated_new_capacity *= 2;
    }
    m_capacity = l_calculated_new_capacity;
  };
};

template <typename T> struct pool {
  pool_intrusive m_intrusive;
  T *m_data;

  uimax &count() { return m_intrusive.m_count; };

  void allocate(uimax p_capacity) {
    m_intrusive.allocate(p_capacity);
    m_data = (T *)sys::malloc(sizeof(*m_data) * m_intrusive.m_capacity);
  };

  void free() {
    m_intrusive.free();
    sys::free(m_data);
  };

  uimax push_back(const T &p_value) {
    uimax l_index;
    if (m_intrusive.find_next_realloc(&l_index)) {
      __realloc();
    }

    at(l_index) = p_value;
    return l_index;
  };

  T &at(uimax p_index) {
    assert_debug(p_index < m_intrusive.m_count);
    assert_debug(m_intrusive.is_element_allocated(p_index));

    return m_data[p_index];
  };

  void remove_at(uimax p_index) {
    assert_debug(m_intrusive.is_element_allocated(p_index));
    m_intrusive.free_element(p_index);
  };

private:
  void __realloc() {
    m_data =
        (T *)sys::realloc(m_data, sizeof(*m_data) * m_intrusive.m_capacity);
  };
};

struct heap_chunk {
  uimax m_begin;
  uimax m_size;
};

struct heap_paged_chunk {
  uimax m_page_index;
  heap_chunk m_chunk;
};

namespace heap_chunks {
static inline ui8 find_next_block(const range<heap_chunk> &p_chunks,
                                  uimax p_size, uimax *out_chunk_index) {
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
  uimax m_single_page_capacity;
  uimax m_element_count;
  vector<heap_chunk> m_free_chunks;
  pool<heap_chunk> m_allocated_chunk;

  enum class state { Undefined = 0, NewChunkPushed = 1 } m_state;

  uimax &count() { return m_element_count; };

  void allocate(uimax p_chunk_size) {
    m_single_page_capacity = p_chunk_size;
    m_free_chunks.allocate(0);
    m_allocated_chunk.allocate(0);
    m_element_count = 0;
    m_state = state::Undefined;
  };

  void free() {
    m_free_chunks.free();
    m_allocated_chunk.free();
  };

  void clear_state() { m_state = state::Undefined; };

  uimax find_next_chunk(uimax p_size) {
    uimax l_chunk_index = -1;
    if (!heap_chunks::find_next_block(m_free_chunks.range(), p_size,
                                      &l_chunk_index)) {
      heap_chunks::defragment(m_free_chunks);
      if (!heap_chunks::find_next_block(m_free_chunks.range(), p_size,
                                        &l_chunk_index)) {
        __push_new_chunk();
        m_state = state::NewChunkPushed;
        heap_chunks::find_next_block(m_free_chunks.range(), p_size,
                                     &l_chunk_index);
      }
    }
    assert_debug(l_chunk_index != -1);
    return l_chunk_index;
  };

  uimax push_found_chunk(uimax p_size, uimax p_free_chunk_indexs) {
    heap_chunk &l_free_chunk = m_free_chunks.at(p_free_chunk_indexs);
    heap_chunk l_chunk;
    l_chunk.m_begin = l_free_chunk.m_begin;
    l_chunk.m_size = p_size;
    uimax l_allocated_chunk_index = m_allocated_chunk.push_back(l_chunk);
    if (l_free_chunk.m_size > p_size) {
      l_free_chunk.m_size -= p_size;
      l_free_chunk.m_begin += p_size;
    } else {
      m_free_chunks.remove_at(p_free_chunk_indexs);
    }
    return l_allocated_chunk_index;
  };

  void free(uimax p_index) {
    auto &l_chunk = m_allocated_chunk.at(p_index);
    m_free_chunks.push_back(l_chunk);
    m_allocated_chunk.remove_at(p_index);
  };

  uimax get_chunk_index_from_begin(uimax p_begin) {
    for (auto l_chunk_it = 0; m_allocated_chunk.count(); ++l_chunk_it) {
      if (m_allocated_chunk.m_intrusive.is_element_allocated(l_chunk_it)) {
        if (m_allocated_chunk.at(l_chunk_it).m_begin == p_begin) {
          return l_chunk_it;
        }
      }
    }
    return -1;
  };

private:
  void __push_new_chunk() {
    heap_chunk l_chunk;
    l_chunk.m_begin = m_element_count;
    l_chunk.m_size = m_single_page_capacity;
    m_free_chunks.push_back(l_chunk);
    m_element_count += l_chunk.m_size;
  };
};

struct heap_paged_intrusive {
  uimax m_single_page_capacity;
  vector_intrusive m_pages_intrusive;
  heap_chunk *m_pages;
  vector<heap_chunk> *m_free_chunks_by_page;

  pool<heap_paged_chunk> m_allocated_chunks;

  enum class state { Undefined = 0, NewPagePushed = 1 } m_state;

  void clear_state() { m_state = state::Undefined; };

  void allocate(uimax p_page_capacity) {
    m_state = state::Undefined;
    m_single_page_capacity = p_page_capacity;
    m_pages_intrusive.allocate(0);
    m_pages = (decltype(m_pages))sys::malloc(sizeof(*m_pages) *
                                             m_pages_intrusive.m_capacity);
    m_free_chunks_by_page = (decltype(m_free_chunks_by_page))sys::malloc(
        sizeof(*m_free_chunks_by_page) * m_pages_intrusive.m_capacity);

    m_allocated_chunks.allocate(0);
  };

  void free() {
    for (auto i = 0; i < m_pages_intrusive.m_count; ++i) {
      m_free_chunks_by_page[i].free();
    };
    sys::free(m_free_chunks_by_page);
    sys::free(m_pages);

    m_allocated_chunks.free();
  };

  void find_next_chunk(uimax p_size, uimax *out_page_index,
                       uimax *out_chunk_index) {

    uimax l_page_index = -1;
    uimax l_chunk_index = -1;

    for (auto l_page_idx = 0; l_page_idx < m_pages_intrusive.m_count;
         ++l_page_idx) {
      auto &l_free_chunks = m_free_chunks_by_page[l_page_idx];
      if (!heap_chunks::find_next_block(l_free_chunks.range(), p_size,
                                        &l_chunk_index)) {
        heap_chunks::defragment(l_free_chunks);
        if (!heap_chunks::find_next_block(l_free_chunks.range(), p_size,
                                          &l_chunk_index)) {
          continue;
        }
      }
      l_page_index = l_page_idx;
      break;
    }

    if (l_page_index == -1) {
      __push_new_page();
      auto &l_free_chunks =
          m_free_chunks_by_page[m_pages_intrusive.m_count - 1];
      if (heap_chunks::find_next_block(l_free_chunks.range(), p_size,
                                       &l_chunk_index)) {
        l_page_index = m_pages_intrusive.m_count - 1;
      }
    }

    assert_debug(l_page_index != -1);
    assert_debug(l_chunk_index != -1);

    *out_page_index = l_page_index;
    *out_chunk_index = l_chunk_index;
  };

  uimax push_found_chunk(uimax p_size, uimax p_page_index,
                         uimax p_chunk_index) {
    auto &l_free_chunks = m_free_chunks_by_page[p_page_index];
    auto &l_free_chunk = l_free_chunks.at(p_chunk_index);
    heap_chunk l_chunk;
    l_chunk.m_begin = l_free_chunk.m_begin;
    l_chunk.m_size = p_size;
    heap_paged_chunk l_paged_chunk;
    l_paged_chunk.m_chunk = l_chunk;
    l_paged_chunk.m_page_index = p_page_index;
    uimax l_allocated_chunk_index = m_allocated_chunks.push_back(l_paged_chunk);
    if (l_free_chunk.m_size > p_size) {
      l_free_chunk.m_size -= p_size;
      l_free_chunk.m_begin += p_size;
    } else {
      l_free_chunks.remove_at(p_chunk_index);
    }
    return l_allocated_chunk_index;
  };

  void remove_chunk(uimax p_chunk_index) {
    heap_paged_chunk &l_paged_chunk = m_allocated_chunks.at(p_chunk_index);
    m_free_chunks_by_page[l_paged_chunk.m_page_index].push_back(
        l_paged_chunk.m_chunk);
    m_allocated_chunks.remove_at(p_chunk_index);
  };

private:
  void __push_new_page() {
    if (m_pages_intrusive.add_realloc(1)) {
      m_pages = (decltype(m_pages))sys::realloc(
          m_pages, sizeof(*m_pages) * m_pages_intrusive.m_capacity);
      m_free_chunks_by_page = (decltype(m_free_chunks_by_page))sys::realloc(
          m_free_chunks_by_page,
          sizeof(*m_free_chunks_by_page) * m_pages_intrusive.m_capacity);
    }

    vector<heap_chunk> &l_page_chunks =
        m_free_chunks_by_page[m_pages_intrusive.m_count - 1];
    l_page_chunks.allocate(0);
    heap_chunk l_chunk;
    l_chunk.m_begin = 0;
    l_chunk.m_size = m_single_page_capacity;
    l_page_chunks.push_back(l_chunk);
    m_state = state::NewPagePushed;
  };
}; // namespace container

struct heap {
  heap_intrusive m_intrusive;
  span<ui8> m_buffer;

  void allocate(uimax p_chunk_size) {
    m_intrusive.allocate(p_chunk_size);
    m_buffer.allocate(0);
  };

  void free() {
    m_buffer.free();
    m_intrusive.free();
  };

  uimax malloc(uimax p_size) {
    uimax l_chunk_index = m_intrusive.find_next_chunk(p_size);
    if (m_intrusive.m_state == heap_intrusive::state::NewChunkPushed) {
      __push_new_chunk();
      m_intrusive.clear_state();
    }
    m_intrusive.push_found_chunk(p_size, l_chunk_index);
    return l_chunk_index;
  };

  ui8 *at(uimax p_index) {
    return m_buffer.m_data + m_intrusive.m_allocated_chunk.at(p_index).m_begin;
  };

  void free(uimax p_index) { m_intrusive.free(p_index); };

private:
  void __push_new_chunk() {
    m_buffer.realloc(m_buffer.count() + m_intrusive.m_single_page_capacity);
  };
};

struct runtime_buffer {
  uimax m_element_size;
  ui8 *m_data;
  uimax m_byte_capacity;

  void allocate() {
    m_element_size = 0;
    m_byte_capacity = 0;
    m_data = (ui8 *)sys::malloc(m_byte_capacity);
  };

  void free() { sys::free(m_data); };

  void resize(uimax p_count) {
    if ((p_count * m_element_size) > m_byte_capacity) {
      m_byte_capacity = (p_count * m_element_size);
      m_data = (ui8 *)sys::realloc(m_data, m_byte_capacity);
    }
  };

  void resize(uimax p_count, uimax p_element_size) {
    m_element_size = p_element_size;
    if ((p_count * m_element_size) > m_byte_capacity) {
      m_byte_capacity = (p_count * m_element_size);
      m_data = (ui8 *)sys::realloc(m_data, m_byte_capacity);
    }
  };

  ui8 *at(uimax p_index) { return m_data + (m_element_size * p_index); };
};

struct runtime_multiple_buffer {
  runtime_buffer *m_runtime_buffers;

  void allocate() { m_runtime_buffers = (runtime_buffer *)sys::malloc(0); };

  void free(uimax p_col_count) {
    for (auto i = 0; i < p_col_count; ++i) {
      m_runtime_buffers[i].free();
    }
    sys::free(m_runtime_buffers);
  };

  void realloc(uimax p_count) {
    m_runtime_buffers = (runtime_buffer *)sys::realloc(
        m_runtime_buffers, p_count * sizeof(*m_runtime_buffers));
  };

  void realloc_cols(uimax p_count, uimax p_col_count) {
    for (auto i = 0; i < p_col_count; ++i) {
      m_runtime_buffers[i].resize(p_count);
    }
  };
};

struct multi_byte_buffer {

  uimax m_col_count;
  runtime_multiple_buffer m_cols;

  void allocate() {
    m_col_count = 0;
    m_cols.allocate();
  };

  void free() { m_cols.free(m_col_count); };

  ui8 *at(uimax p_col_index, uimax p_index) {
    assert_debug(p_col_index < m_col_count);
    return m_cols.m_runtime_buffers[p_col_index].at(p_index);
  };

  void resize_col_capacity(uimax p_count) {
    if (p_count > m_col_count) {
      m_cols.realloc(p_count);

      for (auto l_col_it = m_col_count; l_col_it < p_count; l_col_it++) {
        m_cols.m_runtime_buffers[l_col_it].allocate();
      }

      m_col_count = p_count;
    }
  };

  runtime_buffer &col(uimax p_index) {
    assert_debug(p_index < m_col_count);
    return m_cols.m_runtime_buffers[p_index];
  };
};

} // namespace container