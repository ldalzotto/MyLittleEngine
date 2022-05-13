
#include <api/c_api.h>

int main() {
  engine_handle l_engine = engine_allocate(800,800);
  while (engine_update(l_engine)) {
  }
  engine_free(l_engine);
};