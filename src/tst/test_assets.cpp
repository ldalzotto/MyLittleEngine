#include <doctest.h>

#include <assets/loader/mesh.hpp>
#include <cor/container.hpp>
#include <cstring>

// TODO -> having a type for vertices.
// Thus, replacing all m::vec<fix32, 3> to vertexpos_t

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
vc 0 0 0
vc 10 10 10
vc 20 20 20
vc 15 15 15
vc 30 30 30
vc 40 40 40
vc 50 50 50
  )"""";

    auto l_compiled_mesh = assets::obj_mesh_loader().compile(
        container::range<ui8>::make((ui8 *)l_obj_str, std::strlen(l_obj_str)));

    container::arr<m::vec<fix32, 3>, 8> l_expected_vertices = {
        m::vec<fix32, 3>{1.000000f, -1.000000f, -1.000000f},
        m::vec<fix32, 3>{1.000000f, -1.000000f, 1.000000f},
        m::vec<fix32, 3>{-1.000000, -1.000000, 1.000000},
        m::vec<fix32, 3>{-1.000000, -1.000000, -1.000000},
        m::vec<fix32, 3>{1.000000, 1.000000, -0.999999},
        m::vec<fix32, 3>{0.999999, 1.000000, 1.000001},
        m::vec<fix32, 3>{-1.000000, 1.000000, 1.000000},
        m::vec<fix32, 3>{-1.000000, 1.000000, -1.000000}};

    REQUIRE(l_expected_vertices.range().is_contained_by(
        l_compiled_mesh.position().range()));

    container::arr<m::vec<ui8, 3>, 7> l_expected_colors = {
        m::vec<ui8, 3>{0, 0, 0},    m::vec<ui8, 3>{10, 10, 10},
        m::vec<ui8, 3>{20, 20, 20}, m::vec<ui8, 3>{15, 15, 15},
        m::vec<ui8, 3>{30, 30, 30}, m::vec<ui8, 3>{40, 40, 40},
        m::vec<ui8, 3>{50, 50, 50},
    };

    REQUIRE(l_expected_colors.range().is_contained_by(
        l_compiled_mesh.color().range()));

    l_compiled_mesh.free();
  }

  // pos, uv, normal
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
  )"""";

    auto l_compiled_mesh = assets::obj_mesh_loader().compile(
        container::range<ui8>::make((ui8 *)l_obj_str, std::strlen(l_obj_str)));

    container::arr<m::vec<fix32, 3>, 8> l_expected_vertices = {
        m::vec<fix32, 3>{1.000000f, -1.000000f, -1.000000f},
        m::vec<fix32, 3>{1.000000f, -1.000000f, 1.000000f},
        m::vec<fix32, 3>{-1.000000, -1.000000, 1.000000},
        m::vec<fix32, 3>{-1.000000, -1.000000, -1.000000},
        m::vec<fix32, 3>{1.000000, 1.000000, -0.999999},
        m::vec<fix32, 3>{0.999999, 1.000000, 1.000001},
        m::vec<fix32, 3>{-1.000000, 1.000000, 1.000000},
        m::vec<fix32, 3>{-1.000000, 1.000000, -1.000000}};

    REQUIRE(l_expected_vertices.range().is_contained_by(
        l_compiled_mesh.position().range()));

    container::arr<m::vec<fix32, 2>, 14> l_expected_uvs = {
        m::vec<fix32, 2>{1.000000, 0.333333},
        m::vec<fix32, 2>{1.000000, 0.666667},
        m::vec<fix32, 2>{0.666667, 0.666667},
        m::vec<fix32, 2>{0.666667, 0.333333},
        m::vec<fix32, 2>{0.666667, 0.000000},
        m::vec<fix32, 2>{0.000000, 0.333333},
        m::vec<fix32, 2>{0.000000, 0.000000},
        m::vec<fix32, 2>{0.333333, 0.000000},
        m::vec<fix32, 2>{0.333333, 1.000000},
        m::vec<fix32, 2>{0.000000, 1.000000},
        m::vec<fix32, 2>{0.000000, 0.666667},
        m::vec<fix32, 2>{0.333333, 0.333333},
        m::vec<fix32, 2>{0.333333, 0.666667},
        m::vec<fix32, 2>{1.000000, 0.000000}};

    REQUIRE(
        l_expected_uvs.range().is_contained_by(l_compiled_mesh.uv().range()));

    container::arr<m::vec<fix32, 3>, 6> l_expected_normals = {
        m::vec<fix32, 3>{0.000000, -1.000000, 0.000000},
        m::vec<fix32, 3>{0.000000, 1.000000, 0.000000},
        m::vec<fix32, 3>{1.000000, 0.000000, 0.000000},
        m::vec<fix32, 3>{-0.000000, 0.000000, 1.000000},
        m::vec<fix32, 3>{-1.000000, -0.000000, -0.000000},
        m::vec<fix32, 3>{0.000000, 0.000000, -1.000000},
    };

    REQUIRE(l_expected_normals.range().is_contained_by(
        l_compiled_mesh.normal().range()));

    l_compiled_mesh.free();
  }

  // faces
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

    auto l_compiled_mesh = assets::obj_mesh_loader().compile(
        container::range<ui8>::make((ui8 *)l_obj_str, std::strlen(l_obj_str)));

    container::arr<container::arr<container::arr<ui32, 4>, 3>, 12>
        l_expected_faces = {container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{2, 1, 1, 0},
                                container::arr<ui32, 4>{3, 2, 1, 0},
                                container::arr<ui32, 4>{4, 3, 1, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{8, 1, 2, 0},
                                container::arr<ui32, 4>{7, 4, 2, 0},
                                container::arr<ui32, 4>{6, 5, 2, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{5, 6, 3, 0},
                                container::arr<ui32, 4>{6, 7, 3, 0},
                                container::arr<ui32, 4>{2, 8, 3, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{6, 8, 4, 0},
                                container::arr<ui32, 4>{7, 5, 4, 0},
                                container::arr<ui32, 4>{3, 4, 4, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{3, 9, 5, 0},
                                container::arr<ui32, 4>{7, 10, 5, 0},
                                container::arr<ui32, 4>{8, 11, 5, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{1, 12, 6, 0},
                                container::arr<ui32, 4>{4, 13, 6, 0},
                                container::arr<ui32, 4>{8, 11, 6, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{1, 4, 1, 0},
                                container::arr<ui32, 4>{2, 1, 1, 0},
                                container::arr<ui32, 4>{4, 3, 1, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{5, 14, 2, 0},
                                container::arr<ui32, 4>{8, 1, 2, 0},
                                container::arr<ui32, 4>{6, 5, 2, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{1, 12, 3, 0},
                                container::arr<ui32, 4>{5, 6, 3, 0},
                                container::arr<ui32, 4>{2, 8, 3, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{2, 12, 4, 0},
                                container::arr<ui32, 4>{6, 8, 4, 0},
                                container::arr<ui32, 4>{3, 4, 4, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{4, 13, 5, 0},
                                container::arr<ui32, 4>{3, 9, 5, 0},
                                container::arr<ui32, 4>{8, 11, 5, 0}},
                            container::arr<container::arr<ui32, 4>, 3>{
                                container::arr<ui32, 4>{5, 6, 6, 0},
                                container::arr<ui32, 4>{1, 12, 6, 0},
                                container::arr<ui32, 4>{8, 11, 6, 0}}};

    REQUIRE(l_expected_faces.range() == l_compiled_mesh.face().range());

    l_compiled_mesh.face().range();
    l_compiled_mesh.free();
  }
};

#include <sys/sys_impl.hpp>