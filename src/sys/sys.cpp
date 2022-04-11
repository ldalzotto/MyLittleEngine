#include <sys/sys.hpp>

#include <cstdlib>

namespace {

void *sys::malloc(ui64 p_size) { return std::malloc(p_size.value); };
void sys::free(void *p_ptr) { std::free(p_ptr); };
void *sys::realloc(void *p_ptr, ui64 p_new_size) {
  return std::realloc(p_ptr, p_new_size.value);
};
} // namespace