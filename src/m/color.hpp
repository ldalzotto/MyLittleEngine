#pragma once

#include <cor/types.hpp>

namespace m {

struct color_packed {
  union {
    struct {
      ui8 a : 8;
      ui8 b : 8;
      ui8 g : 8;
      ui8 r : 8;
    };
    ui32 rgba;
  };
};

}; // namespace m