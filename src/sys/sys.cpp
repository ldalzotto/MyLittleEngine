#include <sys/sys.hpp>

#include <cstdlib>

void *sys::malloc(uimax_t p_size) { return std::malloc(p_size); };
void sys::free(void *p_ptr) { std::free(p_ptr); };
void *sys::realloc(void *p_ptr, uimax_t p_new_size) {
  return std::realloc(p_ptr, p_new_size);
};
void sys::memmove(void *p_dest, void *p_src, uimax_t p_n) {
  std::memmove(p_dest, p_src, p_n);
};
void sys::memcpy(void *p_dest, void *p_src, uimax_t p_n) {
  std::memcpy(p_dest, p_src, p_n);
};
void sys::sassert(bool p_condition) {
  if (!p_condition) {
    std::abort();
  }
};

void sys::abort() { std::abort(); };