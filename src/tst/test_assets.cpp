#include <doctest.h>

#include <assets/loader/mesh_obj.hpp>
#include <cor/container.hpp>
#include <cstring>

TEST_CASE("obj.load") {
#if 0
#endif
  // pos and color
  {
    auto l_obj_str = R""""(
# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 1.000000 -1.000000 -1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 -1.000000 1.000000
v -1.000000 -1.000000 -1.000000
v 1.000000 1.000000 -0.999999
v 0.999999 1.000000 1.000001
v -1.000000 1.000000 1.000000
v -1.000000 1.000000 -1.000000
vt 1.000000 0.333333
vt 1.000000 0.666667
vt 0.666667 0.666667
vt 0.666667 0.333333
vt 0.666667 0.000000
vt 0.000000 0.333333
vt 0.000000 0.000000
vt 0.333333 0.000000
vt 0.333333 1.000000
vt 0.000000 1.000000
vt 0.000000 0.666667
vt 0.333333 0.333333
vt 0.333333 0.666667
vt 1.000000 0.000000
vn 0.000000 -1.000000 0.000000
vn 0.000000 1.000000 0.000000
vn 1.000000 0.000000 0.000000
vn -0.000000 0.000000 1.000000
vn -1.000000 -0.000000 -0.000000
vn 0.000000 0.000000 -1.000000
usemtl Material
s off
f 2/1/1 3/2/1 4/3/1
f 8/1/2 7/4/2 6/5/2
f 5/6/3 6/7/3 2/8/3
f 6/8/4 7/5/4 3/4/4
f 3/9/5 7/10/5 8/11/5
f 1/12/6 4/13/6 8/11/6
f 1/4/1 2/1/1 4/3/1
f 5/14/2 8/1/2 6/5/2
f 1/12/3 5/6/3 2/8/3
f 2/12/4 6/8/4 3/4/4
f 4/13/5 3/9/5 8/11/5
f 5/6/6 1/12/6 8/11/6
  )"""";

    assets::mesh l_mesh = assets::obj_mesh_loader().compile(
        container::range<ui8>::make((ui8 *)l_obj_str, std::strlen(l_obj_str)));

    /*
    24 is the number of indices combinaison

    2/1/1
    3/2/1
    4/3/1
    8/1/2
    7/4/2
    6/5/2
    5/6/3
    6/7/3
    2/8/3
    6/8/4
    7/5/4
    3/4/4
    3/9/5
    7/10/5
    8/11/5
    1/12/6
    4/13/6
    8/11/6
    1/4/1
    5/14/2
    1/12/3
    2/12/4
    4/13/5
    5/6/6
    */

    container::arr<position_t, 8> l_raw_vertices = {
        position_t{1.000000f, -1.000000f, -1.000000f},
        position_t{1.000000f, -1.000000f, 1.000000f},
        position_t{-1.000000, -1.000000, 1.000000},
        position_t{-1.000000, -1.000000, -1.000000},
        position_t{1.000000, 1.000000, -0.999999},
        position_t{0.999999, 1.000000, 1.000001},
        position_t{-1.000000, 1.000000, 1.000000},
        position_t{-1.000000, 1.000000, -1.000000}};

    container::arr<uv_t, 14> l_raw_uvs = {
        uv_t{1.000000, 0.333333}, uv_t{1.000000, 0.666667},
        uv_t{0.666667, 0.666667}, uv_t{0.666667, 0.333333},
        uv_t{0.666667, 0.000000}, uv_t{0.000000, 0.333333},
        uv_t{0.000000, 0.000000}, uv_t{0.333333, 0.000000},
        uv_t{0.333333, 1.000000}, uv_t{0.000000, 1.000000},
        uv_t{0.000000, 0.666667}, uv_t{0.333333, 0.333333},
        uv_t{0.333333, 0.666667}, uv_t{1.000000, 0.000000}};

    container::arr<position_t, 6> l_raw_normals = {
        normal_t{0.000000, -1.000000, 0.000000},
        normal_t{0.000000, 1.000000, 0.000000},
        normal_t{1.000000, 0.000000, 0.000000},
        normal_t{-0.000000, 0.000000, 1.000000},
        normal_t{-1.000000, -0.000000, -0.000000},
        normal_t{0.000000, 0.000000, -1.000000},
    };

    REQUIRE(l_mesh.m_positions.count() == 24);
    REQUIRE(l_mesh.m_uvs.count() == 24);
    REQUIRE(l_mesh.m_normals.count() == 24);
    REQUIRE(l_mesh.m_indices.count() == 12 * 3);

    REQUIRE(l_mesh.m_positions.at(0) == l_raw_vertices.at(2 - 1));
    REQUIRE(l_mesh.m_positions.at(1) == l_raw_vertices.at(3 - 1));
    REQUIRE(l_mesh.m_positions.at(2) == l_raw_vertices.at(4 - 1));
    REQUIRE(l_mesh.m_positions.at(3) == l_raw_vertices.at(8 - 1));
    REQUIRE(l_mesh.m_positions.at(4) == l_raw_vertices.at(7 - 1));
    REQUIRE(l_mesh.m_positions.at(5) == l_raw_vertices.at(6 - 1));
    REQUIRE(l_mesh.m_positions.at(6) == l_raw_vertices.at(5 - 1));
    REQUIRE(l_mesh.m_positions.at(7) == l_raw_vertices.at(6 - 1));
    REQUIRE(l_mesh.m_positions.at(8) == l_raw_vertices.at(2 - 1));
    REQUIRE(l_mesh.m_positions.at(9) == l_raw_vertices.at(6 - 1));
    REQUIRE(l_mesh.m_positions.at(10) == l_raw_vertices.at(7 - 1));
    REQUIRE(l_mesh.m_positions.at(11) == l_raw_vertices.at(3 - 1));
    REQUIRE(l_mesh.m_positions.at(12) == l_raw_vertices.at(3 - 1));
    REQUIRE(l_mesh.m_positions.at(13) == l_raw_vertices.at(7 - 1));
    REQUIRE(l_mesh.m_positions.at(14) == l_raw_vertices.at(8 - 1));
    REQUIRE(l_mesh.m_positions.at(15) == l_raw_vertices.at(1 - 1));
    REQUIRE(l_mesh.m_positions.at(16) == l_raw_vertices.at(4 - 1));
    REQUIRE(l_mesh.m_positions.at(17) == l_raw_vertices.at(8 - 1));
    REQUIRE(l_mesh.m_positions.at(18) == l_raw_vertices.at(1 - 1));
    REQUIRE(l_mesh.m_positions.at(19) == l_raw_vertices.at(5 - 1));
    REQUIRE(l_mesh.m_positions.at(20) == l_raw_vertices.at(1 - 1));
    REQUIRE(l_mesh.m_positions.at(21) == l_raw_vertices.at(2 - 1));
    REQUIRE(l_mesh.m_positions.at(22) == l_raw_vertices.at(4 - 1));
    REQUIRE(l_mesh.m_positions.at(23) == l_raw_vertices.at(5 - 1));

    REQUIRE(l_mesh.m_uvs.at(0) == l_raw_uvs.at(1 - 1));
    REQUIRE(l_mesh.m_uvs.at(1) == l_raw_uvs.at(2 - 1));
    REQUIRE(l_mesh.m_uvs.at(2) == l_raw_uvs.at(3 - 1));
    REQUIRE(l_mesh.m_uvs.at(3) == l_raw_uvs.at(1 - 1));
    REQUIRE(l_mesh.m_uvs.at(4) == l_raw_uvs.at(4 - 1));
    REQUIRE(l_mesh.m_uvs.at(5) == l_raw_uvs.at(5 - 1));
    REQUIRE(l_mesh.m_uvs.at(6) == l_raw_uvs.at(6 - 1));
    REQUIRE(l_mesh.m_uvs.at(7) == l_raw_uvs.at(7 - 1));
    REQUIRE(l_mesh.m_uvs.at(8) == l_raw_uvs.at(8 - 1));
    REQUIRE(l_mesh.m_uvs.at(9) == l_raw_uvs.at(8 - 1));
    REQUIRE(l_mesh.m_uvs.at(10) == l_raw_uvs.at(5 - 1));
    REQUIRE(l_mesh.m_uvs.at(11) == l_raw_uvs.at(4 - 1));
    REQUIRE(l_mesh.m_uvs.at(12) == l_raw_uvs.at(9 - 1));
    REQUIRE(l_mesh.m_uvs.at(13) == l_raw_uvs.at(10 - 1));
    REQUIRE(l_mesh.m_uvs.at(14) == l_raw_uvs.at(11 - 1));
    REQUIRE(l_mesh.m_uvs.at(15) == l_raw_uvs.at(12 - 1));
    REQUIRE(l_mesh.m_uvs.at(16) == l_raw_uvs.at(13 - 1));
    REQUIRE(l_mesh.m_uvs.at(17) == l_raw_uvs.at(11 - 1));
    REQUIRE(l_mesh.m_uvs.at(18) == l_raw_uvs.at(4 - 1));
    REQUIRE(l_mesh.m_uvs.at(19) == l_raw_uvs.at(14 - 1));
    REQUIRE(l_mesh.m_uvs.at(20) == l_raw_uvs.at(12 - 1));
    REQUIRE(l_mesh.m_uvs.at(21) == l_raw_uvs.at(12 - 1));
    REQUIRE(l_mesh.m_uvs.at(22) == l_raw_uvs.at(13 - 1));
    REQUIRE(l_mesh.m_uvs.at(23) == l_raw_uvs.at(6 - 1));

    REQUIRE(l_mesh.m_normals.at(0) == l_raw_normals.at(1 - 1));
    REQUIRE(l_mesh.m_normals.at(1) == l_raw_normals.at(1 - 1));
    REQUIRE(l_mesh.m_normals.at(2) == l_raw_normals.at(1 - 1));
    REQUIRE(l_mesh.m_normals.at(3) == l_raw_normals.at(2 - 1));
    REQUIRE(l_mesh.m_normals.at(4) == l_raw_normals.at(2 - 1));
    REQUIRE(l_mesh.m_normals.at(5) == l_raw_normals.at(2 - 1));
    REQUIRE(l_mesh.m_normals.at(6) == l_raw_normals.at(3 - 1));
    REQUIRE(l_mesh.m_normals.at(7) == l_raw_normals.at(3 - 1));
    REQUIRE(l_mesh.m_normals.at(8) == l_raw_normals.at(3 - 1));
    REQUIRE(l_mesh.m_normals.at(9) == l_raw_normals.at(4 - 1));
    REQUIRE(l_mesh.m_normals.at(10) == l_raw_normals.at(4 - 1));
    REQUIRE(l_mesh.m_normals.at(11) == l_raw_normals.at(4 - 1));
    REQUIRE(l_mesh.m_normals.at(12) == l_raw_normals.at(5 - 1));
    REQUIRE(l_mesh.m_normals.at(13) == l_raw_normals.at(5 - 1));
    REQUIRE(l_mesh.m_normals.at(14) == l_raw_normals.at(5 - 1));
    REQUIRE(l_mesh.m_normals.at(15) == l_raw_normals.at(6 - 1));
    REQUIRE(l_mesh.m_normals.at(16) == l_raw_normals.at(6 - 1));
    REQUIRE(l_mesh.m_normals.at(17) == l_raw_normals.at(6 - 1));
    REQUIRE(l_mesh.m_normals.at(18) == l_raw_normals.at(1 - 1));
    REQUIRE(l_mesh.m_normals.at(19) == l_raw_normals.at(2 - 1));
    REQUIRE(l_mesh.m_normals.at(20) == l_raw_normals.at(3 - 1));
    REQUIRE(l_mesh.m_normals.at(21) == l_raw_normals.at(4 - 1));
    REQUIRE(l_mesh.m_normals.at(22) == l_raw_normals.at(5 - 1));
    REQUIRE(l_mesh.m_normals.at(23) == l_raw_normals.at(6 - 1));

    REQUIRE(l_mesh.m_indices.at(0) == 0);
    REQUIRE(l_mesh.m_indices.at(1) == 1);
    REQUIRE(l_mesh.m_indices.at(2) == 2);
    REQUIRE(l_mesh.m_indices.at(3) == 3);
    REQUIRE(l_mesh.m_indices.at(4) == 4);
    REQUIRE(l_mesh.m_indices.at(5) == 5);
    REQUIRE(l_mesh.m_indices.at(6) == 6);
    REQUIRE(l_mesh.m_indices.at(7) == 7);
    REQUIRE(l_mesh.m_indices.at(8) == 8);
    REQUIRE(l_mesh.m_indices.at(9) == 9);
    REQUIRE(l_mesh.m_indices.at(10) == 10);
    REQUIRE(l_mesh.m_indices.at(11) == 11);
    REQUIRE(l_mesh.m_indices.at(12) == 12);
    REQUIRE(l_mesh.m_indices.at(13) == 13);
    REQUIRE(l_mesh.m_indices.at(14) == 14);
    REQUIRE(l_mesh.m_indices.at(15) == 15);
    REQUIRE(l_mesh.m_indices.at(16) == 16);
    REQUIRE(l_mesh.m_indices.at(17) == 17);
    REQUIRE(l_mesh.m_indices.at(18) == 18);
    REQUIRE(l_mesh.m_indices.at(19) == 0);
    REQUIRE(l_mesh.m_indices.at(20) == 2);
    REQUIRE(l_mesh.m_indices.at(21) == 19);
    REQUIRE(l_mesh.m_indices.at(22) == 3);
    REQUIRE(l_mesh.m_indices.at(23) == 5);
    REQUIRE(l_mesh.m_indices.at(24) == 20);
    REQUIRE(l_mesh.m_indices.at(25) == 6);
    REQUIRE(l_mesh.m_indices.at(26) == 8);
    REQUIRE(l_mesh.m_indices.at(27) == 21);
    REQUIRE(l_mesh.m_indices.at(28) == 9);
    REQUIRE(l_mesh.m_indices.at(29) == 11);
    REQUIRE(l_mesh.m_indices.at(30) == 22);
    REQUIRE(l_mesh.m_indices.at(31) == 12);
    REQUIRE(l_mesh.m_indices.at(32) == 14);
    REQUIRE(l_mesh.m_indices.at(33) == 23);
    REQUIRE(l_mesh.m_indices.at(34) == 15);
    REQUIRE(l_mesh.m_indices.at(35) == 17);

    l_mesh.free();
  }
};

#include <sys/sys_impl.hpp>