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

  static constexpr range make(T *p_begin, uimax p_count) {
    return {.m_begin = p_begin, .m_count = p_count};
  };

  const T *data() const { return (const T *)m_begin; };
  const uimax &count() const { return m_count; };
  uimax &count() { return m_count; };
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
    sys::memcpy(m_begin, p_from.m_begin, p_from.size_of());
  };

  template <typename TT> void copy_from(const range<TT> &p_from) const {
    range l_casted_range = p_from.template cast_to<T>();
    copy_from(l_casted_range);
  };

  void zero() { sys::memset(m_begin, 0, m_count * sizeof(T)); };
  void memset(const T &p_value) {
    sys::memset(m_begin, p_value, m_count * sizeof(T));
  };

  range<T> slide(uimax p_count) const {
    assert_debug(m_count >= p_count);
    return range<T>::make(m_begin + p_count, m_count - p_count);
  };

  range<T> shrink_to(uimax p_count) const {
    assert_debug(m_count >= p_count);
    return range<T>::make(m_begin, p_count);
  };

  void slide_self(uimax p_count) {
    assert_debug(m_count >= p_count);
    m_begin += p_count;
    m_count -= p_count;
  };

  template <typename TT> range &stream(const TT &p_value) {
    copy_from(container::range<const TT>::make(&p_value, 1));
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
    assert_debug(p_other.size_of() >= size_of());
    return sys::memcmp(m_begin, p_other.m_begin, size_of()) == 0;
  };
  ui8 operator==(const range &p_other) const {
    for (auto i = 0; i < count(); ++i) {
      if (!(at(i) == p_other.at(i))) {
        return 0;
      }
    }
    return 1;
  };
};

template <typename T, int N> struct arr {
  T m_data[N];
  container::range<T> range() { return container::range<T>::make(m_data, N); };
  constexpr container::range<T> range() const {
    return container::range<T>::make((T *)m_data, N);
  };

  T *data() { return m_data; };
  const T *data() const { return m_data; };
  static constexpr uimax count() { return N; }
  const T &at(uimax p_index) const {
    assert_debug(p_index < N);
    return m_data[p_index];
  };
  T &at(uimax p_index) {
    assert_debug(p_index < N);
    return m_data[p_index];
  };

  ui8 operator==(const arr &p_other) const {
    for (auto i = 0; i < N; ++i) {
      if (!(at(i) == p_other.at(i))) {
        return 0;
      }
    }
    return 1;
  };

  template <typename... ArrTypes>
  static arr<T, N> concat(const ArrTypes &... p_arrs) {
    return arr_multiple_accumulate<0, ArrTypes...>{}(p_arrs...);
  };

private:
  template <ui8 Count, typename ArrFirst, typename... Arrs>
  struct arr_multiple_accumulate {
    auto operator()(const ArrFirst &p_arr_first, const Arrs &... p_arrs) {
      if constexpr (Count < sizeof...(Arrs)) {
        auto l_next_arrs =
            arr_multiple_accumulate<Count + 1, const Arrs &...>{}(p_arrs...);
        arr<T, ArrFirst::count() + l_next_arrs.count()> l_new_arr;
        l_new_arr.range().copy_from(p_arr_first.range());
        l_new_arr.range()
            .slide(ArrFirst::count())
            .copy_from(l_next_arrs.range());
        return l_new_arr;
      } else {
        return p_arr_first;
      }
    };
  };
};

template <typename OutputType, typename InputType, uimax Size>
static constexpr container::arr<OutputType, Size - 1>
arr_literal(const InputType (&arr)[Size]) {
  container::arr<OutputType, Size - 1> l_value = {0};
  for (auto i = 0; i < Size - 1; ++i) {
    l_value.m_data[i] = arr[i];
  }
  return l_value;
};

template <typename T> struct span {

  using element_type = T;

  T *m_data;
  uimax m_count;

  void allocate(uimax p_count) {
    m_data = (T *)default_allocator::malloc(p_count * sizeof(T));
    m_count = p_count;
  };

  void free() { default_allocator::free(m_data); };

  uimax &count() { return m_count; };
  const uimax &count() const { return m_count; };
  T *&data() { return m_data; };
  const T *&data() const { return m_data; };

  uimax size_of() const { return m_count * sizeof(T); };

  void realloc(uimax p_new_count) {
    m_data = (T *)default_allocator::realloc(m_data, p_new_count * sizeof(T));
    m_count = p_new_count;
  };

  void resize(uimax p_new_count) {
    if (p_new_count > count()) {
      realloc(p_new_count);
    }
  };

public:
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

  const T &at(uimax p_index) const { return ((span *)this)->at(p_index); };

  range<T> range() {
    container::range<T> l_range;
    l_range.m_begin = m_data;
    l_range.m_count = m_count;
    return l_range;
  };

  const container::range<T> range() const { return ((span *)this)->range(); };

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

template <typename T, typename Allocator = default_allocator> struct vector {

  vector_intrusive m_intrusive;
  T *m_data;

  uimax &capacity() { return m_intrusive.m_capacity; };
  uimax &count() { return m_intrusive.m_count; };
  const uimax &count() const { return m_intrusive.m_count; };

  void allocate(uimax p_capacity, Allocator *p_allocator = 0) {
    m_intrusive.allocate(p_capacity);
    m_data = (T *)p_allocator->malloc(sizeof(T) * m_intrusive.m_capacity);
  };

  void free(Allocator *p_allocator = 0) { p_allocator->free(m_data); };

  T &at(uimax p_index) {
    assert_debug(p_index < count());
    return m_data[p_index];
  };

  const T &at(uimax p_index) const { return ((vector *)this)->at(p_index); };

  void insert_at(const T &p_element, uimax p_index,
                 Allocator *p_allocator = 0) {
    assert_debug(p_index <= capacity());
    if (m_intrusive.add_realloc(1)) {
      __realloc(p_allocator);
    }
    uimax l_chunk_count = count() - 1 - p_index;
    sys::memmove_down_t(m_data, p_index, l_chunk_count, 1);
    at(p_index) = p_element;
  };

  void push_back(const T &p_element, Allocator *p_allocator = 0) {
    if (m_intrusive.add_realloc(1)) {
      __realloc(p_allocator);
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
      sys::memmove_up_t(m_data, p_index + 1, 1, count() - (p_index + 1));
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
  void __realloc(Allocator *p_allocator) {
    m_data =
        (T *)p_allocator->realloc(m_data, m_intrusive.m_capacity * sizeof(T));
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
    uimax l_index = push_back();
    at(l_index) = p_value;
    return l_index;
  };

  uimax push_back() {
    uimax l_index;
    if (m_intrusive.find_next_realloc(&l_index)) {
      __realloc();
    }
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

  ui8 has_allocated_elements() { return m_intrusive.has_allocated_elements(); };

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
                                  uimax p_size, uimax p_alignment,
                                  uimax *out_chunk_index,
                                  uimax *out_chunk_alignment_offset) {
  assert_debug(p_size > 0);
  assert_debug(p_alignment > 0);
  for (auto l_chunk_it = 0; l_chunk_it < p_chunks.count(); ++l_chunk_it) {
    const heap_chunk &l_chunk = p_chunks.at(l_chunk_it);
    uimax l_chunk_alignment_offset =
        algorithm::alignment_offset(l_chunk.m_begin, p_alignment);
    if ((l_chunk.m_size > l_chunk_alignment_offset) &&
        ((l_chunk.m_size - l_chunk_alignment_offset) >= p_size)) {
      *out_chunk_index = l_chunk_it;
      *out_chunk_alignment_offset = l_chunk_alignment_offset;
      return 1;
    }
  }
  return 0;
};

static inline void defragment(vector<heap_chunk> &p_chunks) {
  if (p_chunks.count() > 0) {
    auto l_range = p_chunks.range();
    ::algorithm::sort(l_range, [&](heap_chunk &p_left, heap_chunk &p_right) {
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
  uimax m_element_count;
  vector<heap_chunk> m_free_chunks;
  pool<heap_chunk> m_allocated_chunk;

  enum class state { Undefined = 0, NewChunkPushed = 1 } m_state;
  uimax m_last_pushed_chunk_size;

  void allocate() {
    m_free_chunks.allocate(0);
    m_allocated_chunk.allocate(0);
    m_element_count = 0;
    m_state = state::Undefined;
    m_last_pushed_chunk_size = 0;
  };

  void allocate(heap_chunk p_free_chunk) {
    m_free_chunks.allocate(1);
    m_free_chunks.push_back(p_free_chunk);
    m_allocated_chunk.allocate(0);
    m_element_count = p_free_chunk.m_size;
    m_state = state::Undefined;
    m_last_pushed_chunk_size = 0;
  };

  void free() {
    m_free_chunks.free();
    m_allocated_chunk.free();
  };

  void clear_state() {
    m_state = state::Undefined;
    m_last_pushed_chunk_size = 0;
  };

  uimax find_next_chunk(uimax p_size) {
    uimax l_chunk_index = -1;
    uimax l_alignment_offset = -1;
    if (!heap_chunks::find_next_block(m_free_chunks.range(), p_size, 1,
                                      &l_chunk_index, &l_alignment_offset)) {
      heap_chunks::defragment(m_free_chunks);
      if (!heap_chunks::find_next_block(m_free_chunks.range(), p_size, 1,
                                        &l_chunk_index, &l_alignment_offset)) {
        // TODO -> pushing a chunk by multiplying capacity by 2 like vector ?
        __push_new_chunk(p_size);

        m_state = state::NewChunkPushed;
        m_last_pushed_chunk_size = p_size;
        heap_chunks::find_next_block(m_free_chunks.range(), p_size, 1,
                                     &l_chunk_index, &l_alignment_offset);
      }
    }
    assert_debug(l_chunk_index != -1);
    return l_chunk_index;
  };

  uimax find_next_chunk(uimax p_size, uimax p_alignment) {
    uimax l_chunk_index = -1;
    uimax l_alignment_offset = -1;
    if (!heap_chunks::find_next_block(m_free_chunks.range(), p_size,
                                      p_alignment, &l_chunk_index,
                                      &l_alignment_offset)) {
      heap_chunks::defragment(m_free_chunks);
      if (!heap_chunks::find_next_block(m_free_chunks.range(), p_size,
                                        p_alignment, &l_chunk_index,
                                        &l_alignment_offset)) {
        // TODO -> pushing a chunk by multiplying capacity by 2 like vector ?
        __push_new_chunk(p_size);

        m_state = state::NewChunkPushed;
        m_last_pushed_chunk_size = p_size;
        heap_chunks::find_next_block(m_free_chunks.range(), p_size, p_alignment,
                                     &l_chunk_index, &l_alignment_offset);
      }
    }
    assert_debug(l_chunk_index != -1);

    if (l_alignment_offset != -1) {
      __split_free_chunk(l_chunk_index, l_alignment_offset);
      l_chunk_index += 1;
    }

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
  void __push_new_chunk_with_offset(uimax p_desired_size,
                                    uimax p_alignment_offset) {
    assert_debug(p_alignment_offset > 0);
    __push_new_chunk(p_alignment_offset);
    __push_new_chunk(p_desired_size);
  };

  void __push_new_chunk(uimax p_desired_size) {
    heap_chunk l_chunk;
    l_chunk.m_begin = m_element_count;
    l_chunk.m_size = p_desired_size;
    m_free_chunks.push_back(l_chunk);
    m_element_count += l_chunk.m_size;
  };

  void __split_free_chunk(uimax p_chunk_index, uimax p_relative_begin) {
    heap_chunk &l_chunk_to_split = m_free_chunks.at(p_chunk_index);
    heap_chunk l_chunk_end = l_chunk_to_split;
    l_chunk_end.m_begin += p_relative_begin;
    l_chunk_end.m_size -= p_relative_begin;

    l_chunk_to_split.m_size = p_relative_begin;
    m_free_chunks.push_back(l_chunk_end);
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

    assert_debug(p_size <= m_single_page_capacity);

    uimax l_page_index = -1;
    uimax l_chunk_index = -1;
    uimax l_alignment_offset = -1;

    for (auto l_page_idx = 0; l_page_idx < m_pages_intrusive.m_count;
         ++l_page_idx) {
      auto &l_free_chunks = m_free_chunks_by_page[l_page_idx];
      if (!heap_chunks::find_next_block(l_free_chunks.range(), p_size, 1,
                                        &l_chunk_index, &l_alignment_offset)) {
        heap_chunks::defragment(l_free_chunks);
        if (!heap_chunks::find_next_block(l_free_chunks.range(), p_size, 1,
                                          &l_chunk_index,
                                          &l_alignment_offset)) {
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
      if (heap_chunks::find_next_block(l_free_chunks.range(), p_size, 1,
                                       &l_chunk_index, &l_alignment_offset)) {
        l_page_index = m_pages_intrusive.m_count - 1;
      }
    }

    assert_debug(l_page_index != -1);
    assert_debug(l_chunk_index != -1);

    *out_page_index = l_page_index;
    *out_chunk_index = l_chunk_index;
  };

  void find_next_chunk(uimax p_size, uimax p_alignment, uimax *out_page_index,
                       uimax *out_chunk_index) {

    assert_debug(p_size <= m_single_page_capacity);

    uimax l_page_index = -1;
    uimax l_chunk_index = -1;
    uimax l_alignment_offset = -1;

    for (auto l_page_idx = 0; l_page_idx < m_pages_intrusive.m_count;
         ++l_page_idx) {
      auto &l_free_chunks = m_free_chunks_by_page[l_page_idx];
      if (!heap_chunks::find_next_block(l_free_chunks.range(), p_size,
                                        p_alignment, &l_chunk_index,
                                        &l_alignment_offset)) {
        heap_chunks::defragment(l_free_chunks);
        if (!heap_chunks::find_next_block(l_free_chunks.range(), p_size,
                                          p_alignment, &l_chunk_index,
                                          &l_alignment_offset)) {
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
                                       p_alignment, &l_chunk_index,
                                       &l_alignment_offset)) {
        l_page_index = m_pages_intrusive.m_count - 1;
      }
    }

    assert_debug(l_page_index != -1);
    assert_debug(l_chunk_index != -1);

    if (l_alignment_offset != -1 && l_alignment_offset != 0) {
      __split_free_chunk(l_page_index, l_chunk_index, l_alignment_offset);
      l_chunk_index += 1;
    }

    *out_page_index = l_page_index;
    *out_chunk_index = l_chunk_index;
  };

  uimax push_found_chunk(uimax p_size, uimax p_page_index,
                         uimax p_chunk_index) {
    auto &l_free_chunks = m_free_chunks_by_page[p_page_index];
    auto &l_free_chunk = l_free_chunks.at(p_chunk_index);
    assert_debug(l_free_chunk.m_size >= p_size);
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
    assert_debug(__check_consistency());
    return l_allocated_chunk_index;
  };

  void remove_chunk(uimax p_chunk_index) {
    heap_paged_chunk &l_paged_chunk = m_allocated_chunks.at(p_chunk_index);
    m_free_chunks_by_page[l_paged_chunk.m_page_index].push_back(
        l_paged_chunk.m_chunk);
    m_allocated_chunks.remove_at(p_chunk_index);
    assert_debug(__check_consistency());
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
    l_chunk.m_size = m_single_page_capacity; // TODO -> dynamic size ?
    l_page_chunks.push_back(l_chunk);
    m_state = state::NewPagePushed;
  };

  void __split_free_chunk(uimax p_free_chunk_page, uimax p_free_chunk_index,
                          uimax p_relative_begin) {
    heap_chunk &l_chunk_to_split =
        m_free_chunks_by_page[p_free_chunk_page].at(p_free_chunk_index);
    heap_chunk l_chunk_end = l_chunk_to_split;
    l_chunk_end.m_begin += p_relative_begin;
    l_chunk_end.m_size -= p_relative_begin;

    l_chunk_to_split.m_size = p_relative_begin;
    m_free_chunks_by_page[p_free_chunk_page].push_back(l_chunk_end);
  };

  ui8 __check_consistency() {

    for (auto l_page_idx = 0; l_page_idx < m_pages_intrusive.m_count;
         ++l_page_idx) {

      for (auto l_allocated_chunk_idx = 0;
           l_allocated_chunk_idx < m_allocated_chunks.count();
           ++l_allocated_chunk_idx) {
        if (m_allocated_chunks.m_intrusive.is_element_allocated(
                l_allocated_chunk_idx)) {
          auto &l_left_chunk = m_allocated_chunks.at(l_allocated_chunk_idx);
          if (l_left_chunk.m_page_index == l_page_idx) {
            uimax l_left_begin = l_left_chunk.m_chunk.m_begin;
            uimax l_left_end =
                l_left_chunk.m_chunk.m_begin + l_left_chunk.m_chunk.m_size;

            for (auto l_right_allocated_chunk_idx = l_allocated_chunk_idx + 1;
                 l_right_allocated_chunk_idx < m_allocated_chunks.count();
                 ++l_right_allocated_chunk_idx) {
              if (m_allocated_chunks.m_intrusive.is_element_allocated(
                      l_right_allocated_chunk_idx)) {
                auto &l_right_chunk =
                    m_allocated_chunks.at(l_right_allocated_chunk_idx);
                if (l_right_chunk.m_page_index == l_page_idx) {
                  uimax l_right_begin = l_right_chunk.m_chunk.m_begin;
                  uimax l_right_end = l_right_chunk.m_chunk.m_begin +
                                      l_right_chunk.m_chunk.m_size;

                  if ((l_right_begin - l_left_begin) > 0 &&
                      (l_right_begin - l_left_end) < 0) {
                    return 0;
                  }
                  if ((l_right_end - l_left_begin) > 0 &&
                      (l_right_end - l_left_end) < 0) {
                    return 0;
                  }
                }
              }
            }

            auto &l_free_chunks = m_free_chunks_by_page[l_page_idx];
            for (auto l_right_free_chunk_idx = 0;
                 l_right_free_chunk_idx < l_free_chunks.count();
                 ++l_right_free_chunk_idx) {
              auto &l_right_chunk = l_free_chunks.at(l_right_free_chunk_idx);

              uimax l_right_begin = l_right_chunk.m_begin;
              uimax l_right_end = l_right_chunk.m_begin + l_right_chunk.m_size;

              if ((l_right_begin - l_left_begin) > 0 &&
                  (l_right_begin - l_left_end) < 0) {
                return 0;
              }
              if ((l_right_end - l_left_begin) > 0 &&
                  (l_right_end - l_left_end) < 0) {
                return 0;
              }
            }
          }
        }
      }
    }

    return 1;
  };
};

struct heap_stacked_intrusive {
  container::vector<heap_chunk> m_allocated_chunks;
  uimax m_cursor;
  uimax m_capacity;

  void allocate(uimax p_capacity) {
    m_allocated_chunks.allocate(0);
    m_capacity = p_capacity;
    clear();
  };

  void clear() {
    m_allocated_chunks.clear();
    m_cursor = 0;
  };

  void increase_capacity(uimax p_delta) {
    uimax l_new_capacity = m_capacity + p_delta;
    while (m_capacity < l_new_capacity) {
      if (m_capacity == 0) {
        m_capacity = 1;
      } else {
        m_capacity *= 2;
      }
    }
  };

  void free() { m_allocated_chunks.free(); };

  uimax count() { return m_allocated_chunks.count(); };
  ui8 has_allocated_elements() { return count() != 0; };

  ui8 find_next_chunk(uimax p_size, uimax p_alignment, uimax *out_chunk_index) {
    uimax l_alignement_offset =
        algorithm::alignment_offset(m_cursor, p_alignment);
    uimax l_total_size = p_size + l_alignement_offset;
    if ((m_cursor + l_total_size) > m_capacity) {
      return 0;
    }

    heap_chunk l_chunk;
    l_chunk.m_begin = m_cursor + l_alignement_offset;
    l_chunk.m_size = p_size;
    m_cursor += l_total_size;
    m_allocated_chunks.push_back(l_chunk);
    *out_chunk_index = m_allocated_chunks.count() - 1;

    return 1;
  };

  ui8 consistency() { return m_cursor <= m_capacity; };
};

struct heap {
  heap_intrusive m_intrusive;
  span<ui8> m_buffer;

  void allocate() {
    m_intrusive.allocate();
    m_buffer.allocate(0);
  };

  void allocate(uimax p_buffer_size) {
    m_buffer.allocate(p_buffer_size);
    heap_chunk l_free_chunk;
    l_free_chunk.m_begin = 0;
    l_free_chunk.m_size = p_buffer_size;
    m_intrusive.allocate(l_free_chunk);
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
    return m_intrusive.push_found_chunk(p_size, l_chunk_index);
  };

  ui8 *at(uimax p_index) {
    return m_buffer.m_data + m_intrusive.m_allocated_chunk.at(p_index).m_begin;
  };

  container::range<ui8> range(uimax p_index) {
    heap_chunk &l_chunk = m_intrusive.m_allocated_chunk.at(p_index);
    return container::range<ui8>::make(m_buffer.data() + l_chunk.m_begin,
                                       l_chunk.m_size);
  };

  void free(uimax p_index) { m_intrusive.free(p_index); };

private:
  void __push_new_chunk() {
    m_buffer.realloc(m_buffer.count() + m_intrusive.m_last_pushed_chunk_size);
  };
};

template <typename T> struct heap_paged {
  heap_paged_intrusive m_intrusive;
  T **m_data;

  void allocate(uimax p_page_capacity) {
    m_intrusive.allocate(p_page_capacity);
    m_data = (T **)sys::malloc(0);
  };

  void free() {
    for (auto i = 0; i < m_intrusive.m_pages_intrusive.m_count; ++i) {
      sys::free(m_data[i]);
    }
    sys::free(m_data);
  };

  uimax push_back(uimax p_size) {
    uimax l_page_index, l_chunk_index;
    m_intrusive.find_next_chunk(p_size, &l_page_index, &l_chunk_index);
    if (m_intrusive.m_state ==
        container::heap_paged_intrusive::state::NewPagePushed) {
      m_intrusive.clear_state();
      this->realloc();
      this->allocate_page(l_page_index);
    };

    return m_intrusive.push_found_chunk(p_size, l_page_index, l_chunk_index);
  };

  container::range<T> at(uimax p_chunk_index) {
    return map_to_range(m_intrusive.m_allocated_chunks.at(p_chunk_index));
  };

  void remove_at(uimax p_chunk_index) {
    m_intrusive.remove_chunk(p_chunk_index);
  };

private:
  void realloc() {
    m_data = (T **)sys::realloc(
        m_data, m_intrusive.m_pages_intrusive.m_capacity * sizeof(*m_data));
  };

  void allocate_page(uimax p_page_index) {
    m_data[p_page_index] =
        (T *)sys::malloc(sizeof(T) * m_intrusive.m_single_page_capacity);
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

struct heap_paged_allocator {
  using header_t = uimax;
  heap_paged<ui8> m_heap_paged;
  void allocate(uimax p_page_capacity) {
    m_heap_paged.allocate(p_page_capacity);
  };

  void free() { m_heap_paged.free(); };

  void *malloc(uimax p_size) {
    uimax l_chunk = m_heap_paged.push_back(sizeof(header_t) + p_size);
    container::range<ui8> l_range = m_heap_paged.at(l_chunk);
    *(header_t *)&l_range.at(0) = l_chunk;
    l_range.slide_self(sizeof(header_t));
    return l_range.m_begin;
  };

  void free(void *p_ptr) {
    ui8 *l_ptr = (ui8 *)p_ptr;
    header_t *l_header = (header_t *)(l_ptr - sizeof(header_t));
    m_heap_paged.remove_at(*l_header);
  };

  void *realloc(void *p_ptr, uimax p_new_size) {
    ui8 *l_new = (ui8 *)malloc(p_new_size);
    ui8 *l_old = (ui8 *)p_ptr;
    header_t *l_old_header = (header_t *)(l_old - sizeof(header_t));
    container::range<ui8> l_old_range = m_heap_paged.at(*l_old_header);
    sys::memcpy(l_new, l_old, l_old_range.count());
    free(p_ptr);
    return l_new;
  };
};

struct heap_stacked {
  heap_stacked_intrusive m_intrusive;
  container::span<ui8> m_data;

  void allocate(uimax p_capacity) {
    m_intrusive.allocate(p_capacity);
    m_data.allocate(p_capacity);
  };

  void free() {
    m_intrusive.free();
    m_data.free();
  };

  void clear() { m_intrusive.clear(); };

  void push_back(uimax p_size, uimax p_alignment) {
    uimax l_chunk_index = -1;
    if (!m_intrusive.find_next_chunk(p_size, p_alignment, &l_chunk_index)) {
      m_intrusive.increase_capacity(p_size + p_alignment);
      m_data.realloc(m_intrusive.m_capacity);
      m_intrusive.find_next_chunk(p_size, p_alignment, &l_chunk_index);
    }
    assert_debug(m_intrusive.consistency());
    assert_debug(l_chunk_index != -1);
  };

  void push_back_no_realloc(uimax p_size, uimax p_alignment) {
    uimax l_chunk_index = -1;
    m_intrusive.find_next_chunk(p_size, p_alignment, &l_chunk_index);
    assert_debug(l_chunk_index != -1);
  };

  uimax count() { return m_intrusive.count(); };
  ui8 has_allocated_elements() { return m_intrusive.has_allocated_elements(); };

  container::range<ui8> at(uimax p_index) {
    assert_debug(p_index < count());
    heap_chunk &l_chunk = m_intrusive.m_allocated_chunks.at(p_index);
    return container::range<ui8>::make(m_data.m_data + l_chunk.m_begin,
                                       l_chunk.m_size);
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

template <typename Key> struct hashmap_intrusive {
  container::pool_intrusive m_keys_intrisic;
  Key *m_keys;
  ui8 *m_is_allocated;

  void allocate() {
    m_keys_intrisic.allocate(0);
    m_keys = (Key *)default_allocator::malloc(0);
    m_is_allocated = (ui8 *)default_allocator::malloc(0);
  };

  void free() {
    m_keys_intrisic.free();
    default_allocator::free(m_keys);
    default_allocator::free(m_is_allocated);
  };

  ui8 push_back_realloc(const Key &p_key, uimax *out_index) {
    assert_debug(find_key_index(p_key) == -1);

    ui8 l_needs_reallocate = 0;
    uimax l_old_capacity = m_keys_intrisic.m_capacity;
    uimax l_index;
    if (m_keys_intrisic.find_next_realloc(&l_index)) {
      __realloc(m_keys_intrisic.m_capacity);
      for (auto i = l_old_capacity; i < m_keys_intrisic.m_capacity; ++i) {
        m_is_allocated[i] = 0;
      }
      l_needs_reallocate = 1;
    }
    m_keys[l_index] = p_key;
    m_is_allocated[l_index] = 1;
    *out_index = l_index;
    return l_needs_reallocate;
  };

  void remove_at(const Key &p_key) {
    uimax l_index = find_key_index(p_key);
    m_keys_intrisic.free_element(l_index);
    m_is_allocated[l_index] = 0;
  };

  ui8 has_key(const Key &p_key) const { return find_key_index(p_key) != -1; };

  uimax find_key_index(const Key &p_key) const {
    for (auto i = 0; i < m_keys_intrisic.m_count; ++i) {
      if (m_is_allocated[i]) {
        if (m_keys[i] == p_key) {
          return i;
        }
      }
    }
    return -1;
  };

  ui8 has_allocated_elements() const {
    ui8 l_has_allocated_elements = 0;
    return m_keys_intrisic.m_free_elements.count() != m_keys_intrisic.m_count;
  };

private:
  void __realloc(uimax p_new_size) {
    m_keys =
        (Key *)default_allocator::realloc(m_keys, sizeof(*m_keys) * p_new_size);
    m_is_allocated = (ui8 *)default_allocator::realloc(
        m_is_allocated, sizeof(*m_is_allocated) * p_new_size);
  };
};

template <typename Key, typename Value> struct hashmap {
  hashmap_intrusive<Key> m_intrusive;
  Value *m_data;

  void allocate() {
    m_intrusive.allocate();
    m_data = (Value *)default_allocator::malloc(0);
  };

  void free() {
    m_intrusive.free();
    default_allocator::free(m_data);
  };

  void push_back(const Key &p_key, const Value &p_value) {
    uimax l_index;
    if (m_intrusive.push_back_realloc(p_key, &l_index)) {
      __realloc();
    }
    m_data[l_index] = p_value;
  };

  void remove_at(const Key &p_key) { m_intrusive.remove_at(p_key); };

  Value &at(const Key &p_key) {
    uimax l_index = m_intrusive.find_key_index(p_key);
    assert_debug(l_index != -1);
    return m_data[l_index];
  };

  ui8 has_key(const Key &p_key) const { return m_intrusive.has_key(p_key); };

  ui8 has_allocated_elements() const {
    return m_intrusive.has_allocated_elements();
  };

private:
  void __realloc() {
    m_data = (Value *)default_allocator::realloc(
        m_data, sizeof(*m_data) * m_intrusive.m_keys_intrisic.m_capacity);
  };
};

namespace traits {
template <typename T> struct is_range {
  inline static constexpr ui8 value = 0;
};
template <typename T> struct is_range<range<T>> {
  inline static constexpr ui8 value = 1;
};
}; // namespace traits

} // namespace container