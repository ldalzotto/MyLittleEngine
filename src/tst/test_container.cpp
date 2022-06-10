
#include <doctest.h>

#include <cor/container.hpp>

TEST_CASE("container.test") {
  container::heap_paged_allocator l_heap_paged_allocator;
  l_heap_paged_allocator.allocate(1024);

  container::vector<uimax, container::heap_paged_allocator> l_v;
  l_v.allocate(10, &l_heap_paged_allocator);
  l_v.push_back(10);
  REQUIRE(l_v.at(0) == 10);
  l_v.free(&l_heap_paged_allocator);

  l_heap_paged_allocator.free();
}