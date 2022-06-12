
#include <doctest.h>

#include <cor/container.hpp>
#include <cor/orm.hpp>

TEST_CASE("container.test") {
  container::heap_paged_allocator l_heap_paged_allocator;
  l_heap_paged_allocator.allocate(1024);

  container::vector<uimax, container::heap_paged_allocator> l_v;
  l_v.allocate(10, &l_heap_paged_allocator);
  l_v.push_back(10);
  REQUIRE(l_v.at(0) == 10);
  l_v.free(&l_heap_paged_allocator);

  l_heap_paged_allocator.free();

  orm::table_span_v2<ui8, ui8, orm::details::one_to_many_col> l_resource_a;
  orm::table_span_v2<ui8> l_resource_b;
  static constexpr ui8 resource_a_first = 0;
  static constexpr ui8 resource_a_second = 1;
  static constexpr ui8 resource_b_first = 0;

  l_resource_a.allocate(10);
  l_resource_b.allocate(1);

  l_resource_a.set(0, 0, none());
  l_resource_a.rel_allocate<2>(0, 0);

  l_resource_b.set(0, 2);
  l_resource_a.rel<2>(0).push(0);

  orm::one_to_many l_res_a_to_b = l_resource_a.rel<2>(0);

  l_resource_a.free();
  l_resource_b.free();
};