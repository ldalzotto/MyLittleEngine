#pragma once

#include <m/vec.hpp>
#include <m/quat.hpp>

using position_t = m::vec<fix32, 3>;
using rotation_t = m::quat<fix32>;
using uv_t = m::vec<fix32, 2>;
using rgb_t = m::vec<ui8, 3>;
using rgba_t = m::vec<ui8, 4>;
using rgbf_t = m::vec<fix32, 3>;
using rgbaf_t = m::vec<fix32, 4>;
using normal_t = m::vec<fix32, 3>;
using vindex_t = ui16;