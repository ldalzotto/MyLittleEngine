#pragma once

#include <cor/types.hpp>

struct byte_struct_0 {

  using member_0 = f32;
  using member_1 = ui8;

  uimax byte_size() { return sizeof(member_0) + sizeof(member_1); };

  struct view {
    ui8 *m_data;

    member_0 &mem0() { return *(member_0 *)m_data; };
    member_1 &mem1() { return *(member_1 *)(m_data + sizeof(member_0)); };
  };
};