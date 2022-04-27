#pragma once

#include <cor/types.hpp>

namespace m {

template <typename T, int C, int L> struct mat { T m_data[C * L]; };

} // namespace m