
#include <cor/v2.hpp>
#include <tst/test_common.hpp>

using namespace v2;

container_declare_alias_2(text_data, i32, m_ints, f32, m_flts);

TEST_CASE("core") {

  span_1<int> l_span_1;
  span_allocate(&l_span_1, 10);

  span_2<int, float> l_span_2;
  span_allocate(&l_span_2, 10);

  slice_1<int> l_slice_1 = span_to_slice(&l_span_1);
  slice_1<int> l_slice_another = span_to_slice(&l_span_1);

  slice_copy_to(&l_slice_1, &l_slice_another);
  slice_zero(&l_slice_another);

  slice_2<int, float> l_slice_2 = span_to_slice(&l_span_2);
  slice_zero(&l_slice_2);

  vector_2<int, float> l_vector_2;
  vector_allocate(&l_vector_2, 0);
  vector_insert_at(&l_vector_2, 0, 5);
  vector_remove_at(&l_vector_2, 0, 5);

  auto l_vector_2_slice = vector_to_slice(&l_vector_2);
  slice_slide(&l_vector_2_slice, 0);

  pool_2<int, float> l_pool;
  pool_allocate(&l_pool, 0);
  uimax l_index = pool_push(&l_pool);
  l_pool.m_elements.m_data.m_0[l_index] = 10;
  pool_remove(&l_pool, l_index);
  l_index = pool_push(&l_pool);
  pool_free(&l_pool);

  for (uimax i = 0; i < l_slice_2.m_count; ++i) {
    l_slice_2.m_data.m_0[i] = i;
    l_slice_2.m_data.m_1[i] = i;
  }

  slice_sort(&l_slice_2,
             [&](tuple<2, i32 *, f32 *> *left, tuple<2, i32 *, f32 *> *right) {
               return *left->m_0 <= *right->m_0;
             });

  span_free(&l_span_1);
  span_free(&l_span_2);
  vector_free(&l_vector_2);

  heap_2<i32, f32> l_heap_2;
  heap_allocate(&l_heap_2);
  heap_push_new_free_chunk(&l_heap_2, 100);
  uimax l_chunk_index = heap_allocate_chunk(&l_heap_2, 10);
  uimax l_chunk_another_index = heap_allocate_chunk(&l_heap_2, 5);
  heap_free_chunk(&l_heap_2, l_chunk_index);
  heap_allocate_chunk(&l_heap_2, 10);
  heap_free(&l_heap_2);
}

#include <sys/sys_impl.hpp>