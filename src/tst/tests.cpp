#include <bgfx/bgfx.h>
#include <cor/container.hpp>
#include <cor/cor.hpp>
#include <cor/orm.hpp>
#include <sys/sys.hpp>

void container_tests() {

  struct entry {
    i32_t m_v0, m_v1, m_v2;
    entry(i32_t p_value) {
      m_v0 = p_value;
      m_v1 = p_value;
      m_v2 = p_value;
    };

    ui8_t operator==(i32_t p_value) {
      return m_v0 == p_value && m_v1 == p_value && m_v2 == p_value;
    };
  };

  container::vector<entry> l_vector;
  l_vector.allocate(0);
  for (uimax_t i = 0; i < 10; ++i) {
    l_vector.push_back(entry(i));
  }
  for (uimax_t i = 0; i < 10; ++i) {
    sys::sassert(l_vector.at(i) == i);
  }

  container::span<entry> l_span;
  l_span.allocate(1);

  l_vector.insert_at(l_span.range(), 0);
  l_vector.free();

  {
    orm::table<orm::table_col_types<i32_t, f32_t>,
               orm::table_memory_layout::VECTOR>
        l_table;
    l_table.allocate(0);
    auto l_index_0 = l_table.push_back(2, 10);
    auto l_index_1 = l_table.push_back(3, 11);
    auto l_index_2 = l_table.push_back(4, 12);

    sys::sassert(l_table.at<0>(l_index_0) == 2);
    sys::sassert(l_table.at<1>(l_index_0) == 10);
    sys::sassert(l_table.at<0>(l_index_1) == 3);
    sys::sassert(l_table.at<1>(l_index_1) == 11);
    sys::sassert(l_table.at<0>(l_index_2) == 4);
    sys::sassert(l_table.at<1>(l_index_2) == 12);

    l_table.remove_at(l_index_1);

    for (auto l_iter = l_table.iter<0>(); l_iter.next();) {
      l_iter.value() = 10;
    }

    l_table.free();
  }

  {
    orm::table<orm::table_col_types<i32_t, i32_t>,
               orm::table_memory_layout::POOL>
        l_table;
    l_table.allocate(0);
    auto l_index_0 = l_table.push_back(2, 10);
    auto l_index_1 = l_table.push_back(3, 11);
    auto l_index_2 = l_table.push_back(4, 12);

    sys::sassert(l_table.at<0>(l_index_0) == 2);
    sys::sassert(l_table.at<1>(l_index_0) == 10);
    sys::sassert(l_table.at<0>(l_index_1) == 3);
    sys::sassert(l_table.at<1>(l_index_1) == 11);
    sys::sassert(l_table.at<0>(l_index_2) == 4);
    sys::sassert(l_table.at<1>(l_index_2) == 12);

    l_table.remove_at(l_index_1);

    l_table.free();
  }

  {
    orm::table<orm::table_col_types<i32_t, i32_t, ui8_t>,
               orm::table_memory_layout::POOL_FIXED>
        l_table;
    l_table.allocate(1000);
    auto l_index_0 = l_table.push_back(2, 10, 5);
    auto l_index_1 = l_table.push_back(3, 11, 6);
    auto l_index_2 = l_table.push_back(4, 12, 7);

    static const ui8_t first = 0;
    static const ui8_t second = 1;
    static const ui8_t third = 2;

    sys::sassert(l_table.at<first>(l_index_0) == 2);
    sys::sassert(l_table.at<second>(l_index_0) == 10);
    sys::sassert(l_table.at<first>(l_index_1) == 3);
    sys::sassert(l_table.at<second>(l_index_1) == 11);
    sys::sassert(l_table.at<first>(l_index_2) == 4);
    sys::sassert(l_table.at<second>(l_index_2) == 12);

    l_table.remove_at(l_index_1);

    l_table.free();
  }

  {
    orm::table<orm::table_col_types<i32_t, i32_t>,
               orm::table_memory_layout::HEAP_FIXED>
        l_table;
    l_table.allocate(1024);
    auto l_index_0 = l_table.push_back(2, 10);
    auto l_index_1 = l_table.push_back(3, 11);
    auto l_index_2 = l_table.push_back(4, 12);

    sys::sassert(l_table.at<0>(l_index_0) == 2);
    sys::sassert(l_table.at<1>(l_index_0) == 10);
    sys::sassert(l_table.at<0>(l_index_1) == 3);
    sys::sassert(l_table.at<1>(l_index_1) == 11);
    sys::sassert(l_table.at<0>(l_index_2) == 4);
    sys::sassert(l_table.at<1>(l_index_2) == 12);

    auto l_old_index_1 = l_index_1;
    l_table.remove_at(l_index_1);
    l_index_1 = l_table.push_back(3, 11);
    sys::sassert(l_old_index_1 == l_index_1); // Index is the same
    l_table.free();
  }

  {
    orm::table<orm::table_col_types<ui8_t>,
               orm::table_memory_layout::HEAP_FIXED_BYTES>
        l_table;
    l_table.allocate(1024);
    auto l_index_0 = l_table.push_back(2);
    auto l_index_1 = l_table.push_back(3);
    auto l_index_2 = l_table.push_back(4);

    container::range<ui8_t> l_0 = l_table.range<0>(l_index_0);
    container::range<ui8_t> l_1 = l_table.range<0>(l_index_1);
    container::range<ui8_t> l_2 = l_table.range<0>(l_index_2);

    sys::sassert(l_0.m_count == 2);
    sys::sassert(l_1.m_count == 3);
    sys::sassert(l_2.m_count == 4);

    sys::sassert((int)(l_1.m_begin - l_0.m_begin) == 2);
    sys::sassert((int)(l_2.m_begin - l_0.m_begin) == 5);

    auto l_old_index_1 = l_index_1;
    l_table.remove_at(l_index_1);
    l_index_1 = l_table.push_back(3);
    sys::sassert(l_old_index_1 == l_index_1); // Index is the same
    l_table.free();
  }

  {
    using table_0 = orm::table<orm::table_col_types<i32_t, i32_t, ui8_t>,
                               orm::table_memory_layout::POOL_FIXED>;
    using table_1 = orm::table<orm::table_col_types<i32_t, i32_t>,
                               orm::table_memory_layout::VECTOR>;
    orm::db<orm::db_table_types<table_0, table_1>> l_db;
    l_db.allocate(100, 0);
    auto l_index = l_db.push_back<1>(1, 2);
    l_db.push_back<0>(1, 2, 3);

    sys::sassert(l_db.at<1, 0>(l_index) == 1);
    sys::sassert(l_db.at<1, 1>(l_index) == 2);

    l_db.free();
  }
};

struct PosColorVertex {
  float m_x;
  float m_y;
  float m_z;
  uint32_t m_abgr;

  static bgfx::VertexLayout ms_layout;

  static void init() {
    ms_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
  };
};

bgfx::VertexLayout PosColorVertex::ms_layout = bgfx::VertexLayout();

static const uint16_t s_cubeTriList[] = {
    0, 1, 2,          // 0
    1, 3, 2, 4, 6, 5, // 2
    5, 6, 7, 0, 2, 4, // 4
    4, 2, 6, 1, 5, 3, // 6
    5, 7, 3, 0, 4, 1, // 8
    4, 5, 1, 2, 3, 6, // 10
    6, 3, 7,
};

static const uint16_t s_cubeTriStrip[] = {
    0, 1, 2, 3, 7, 1, 5, 0, 4, 2, 6, 7, 4, 5,
};

static const uint16_t s_cubeLineList[] = {
    0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3, 2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7,
};

static const uint16_t s_cubeLineStrip[] = {
    0, 2, 3, 1, 5, 7, 6, 4, 0, 2, 6, 4, 5, 7, 3, 1, 0,
};

static const uint16_t s_cubePoints[] = {0, 1, 2, 3, 4, 5, 6, 7};

static const char *s_ptNames[]{
    "Triangle List", "Triangle Strip", "Lines", "Line Strip", "Points",
};

static const uint64_t s_ptState[]{
    UINT64_C(0),          BGFX_STATE_PT_TRISTRIP,
    BGFX_STATE_PT_LINES,  BGFX_STATE_PT_LINESTRIP,
    BGFX_STATE_PT_POINTS,
};

static constexpr ui8_t s_ptState_count =
    sizeof(s_ptState) / sizeof(s_ptState[0]);

int main() {

  container_tests();

  bgfx::init();

  boost::container::vector<int> zd;

  PosColorVertex::init();
  static PosColorVertex s_cubeVertices[] = {
      {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
      {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
      {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
      {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
  };

  bgfx::VertexBufferHandle m_vbh;
  bgfx::IndexBufferHandle m_ibh[s_ptState_count];

  auto l_frame_buffer =
      bgfx::createFrameBuffer(100, 100, bgfx::TextureFormat::RG8);

  m_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)),
      PosColorVertex::ms_layout);

  // Create static index buffer for triangle list rendering.
  m_ibh[0] = bgfx::createIndexBuffer(
      // Static data can be passed with bgfx::makeRef
      bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));

  // Create static index buffer for triangle strip rendering.
  m_ibh[1] = bgfx::createIndexBuffer(
      // Static data can be passed with bgfx::makeRef
      bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip)));

  // Create static index buffer for line list rendering.
  m_ibh[2] = bgfx::createIndexBuffer(
      // Static data can be passed with bgfx::makeRef
      bgfx::makeRef(s_cubeLineList, sizeof(s_cubeLineList)));

  // Create static index buffer for line strip rendering.
  m_ibh[3] = bgfx::createIndexBuffer(
      // Static data can be passed with bgfx::makeRef
      bgfx::makeRef(s_cubeLineStrip, sizeof(s_cubeLineStrip)));

  // Create static index buffer for point list rendering.
  m_ibh[4] = bgfx::createIndexBuffer(
      // Static data can be passed with bgfx::makeRef
      bgfx::makeRef(s_cubePoints, sizeof(s_cubePoints)));

  bgfx::setViewRect(0, 0, 0, 100, 100);
  mat<f32, 4, 4> viewMatrix, projectionMatrix;
  bgfx::setViewTransform(0, &viewMatrix, &projectionMatrix);
  bgfx::setViewFrameBuffer(0, l_frame_buffer);
  bgfx::touch(0);

  for (i32_t i = 0; i < 10; ++i) {
    mat<f32, 4, 4> transformMatrix;
    bgfx::setTransform(&transformMatrix);

    // Set vertex and index buffer.
    bgfx::setVertexBuffer(0, m_vbh);
    // TODO
    // bgfx::setIndexBuffer(ibh);

    bgfx::submit(0, bgfx::ProgramHandle());
  }
  bgfx::frame();

  // Cleanup.
  for (uint32_t ii = 0; ii < s_ptState_count; ++ii) {
    bgfx::destroy(m_ibh[ii]);
  }

  bgfx::destroy(m_vbh);
  // bgfx::destroy(m_program);

  bgfx::shutdown();

  // TODO -> memleak in containers (maybe as an option ?) extended container
};

#include <rast/rast.hpp>