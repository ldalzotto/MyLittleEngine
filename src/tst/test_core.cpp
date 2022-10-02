
#include <cor/v2.hpp>
#include <tst/test_common.hpp>

TEST_CASE("core") {
  using namespace v2;

  span_1<int> l_span_1;
  span_allocate(&l_span_1, 10);

  span_2<int, float> l_span_2;
  span_allocate(&l_span_2, 10);

  slice_1<int> l_slice_1 = span_to_slice(&l_span_1);
  slice_1<int> l_slice_another = span_to_slice(&l_span_1);

  slice_copy_to(&l_slice_1, &l_slice_another);
  slice_zero(&l_slice_another);

  auto l_slice_2 = span_to_slice(&l_span_2);
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

  span_free(&l_span_1);
  span_free(&l_span_2);
  vector_free(&l_vector_2);
}

#include <sys/sys_impl.hpp>