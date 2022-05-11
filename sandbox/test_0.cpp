
#include <api/c_api.h>

int main() {
  engine_handle l_handle = engine_allocate();
  window_handle l_window = engine_widow_open(l_handle, 1000, 1000);
  while (true) {
    engine_update(l_handle);
  }
  engine_window_close(l_handle, l_window);
  engine_free(l_handle);
};