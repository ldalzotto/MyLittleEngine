#pragma once

#include <m/mat.hpp>
#include <m/quat.hpp>
#include <m/vec.hpp>

namespace m {
template <typename T>
T cross(const vec<T, 2> &p_left, const vec<T, 2> &p_right) {
  return p_left.x() * p_right.y() - p_left.y() * p_right.x();
};

template <typename T>
vec<T, 3> cross(const vec<T, 3> &p_left, const vec<T, 3> &p_right) {
  return vec<T, 3>{p_left.y() * p_right.z() - p_left.z() * p_right.y(),
                   p_left.z() * p_right.x() - p_left.x() * p_right.z(),
                   p_left.x() * p_right.y() - p_left.y() * p_right.x()};
};

template <typename T> T dot(const vec<T, 2> &p_left, const vec<T, 2> &p_right) {
  return (p_left.x() * p_right.x()) + (p_left.y() * p_right.y());
};

template <typename T> T dot(const vec<T, 3> &p_left, const vec<T, 3> &p_right) {
  return (p_left.x() * p_right.x()) + (p_left.y() * p_right.y()) +
         (p_left.z() * p_right.z());
};

template <typename T> fix32 magnitude(const vec<T, 3> &thiz) {
  return m::sqrt(dot(thiz, thiz));
};

template <typename T> vec<T, 3> normalize(const vec<T, 3> &thiz) {
  return thiz / magnitude(thiz);
};

template <typename T> ui8 is_normalized(const vec<T, 3> &thiz) {
  auto l_magnitude = magnitude(thiz);
  return l_magnitude >= 0.99 && l_magnitude <= 1.01;
};

template <typename T>
T perp_dot(const vec<T, 2> &p_left, const vec<T, 2> &p_right) {
  return (p_left.x() * p_right.y()) - (p_left.y() * p_right.x());
};

template <typename T> mat<T, 3, 3> rotation_3(const quat<T> &p_quat) {
  mat<T, 3, 3> l_return = l_return.getIdentity();
  T qxx = p_quat.x() * p_quat.x();
  T qyy = p_quat.y() * p_quat.y();
  T qzz = p_quat.z() * p_quat.z();
  T qxz = p_quat.x() * p_quat.z();
  T qxy = p_quat.x() * p_quat.y();
  T qyz = p_quat.y() * p_quat.z();
  T qwx = p_quat.w() * p_quat.x();
  T qwy = p_quat.w() * p_quat.y();
  T qwz = p_quat.w() * p_quat.z();

  l_return.at(0, 0) = T(1) - T(2) * (qyy + qzz);
  l_return.at(0, 1) = T(2) * (qxy + qwz);
  l_return.at(0, 2) = T(2) * (qxz - qwy);

  l_return.at(1, 0) = T(2) * (qxy - qwz);
  l_return.at(1, 1) = T(1) - T(2) * (qxx + qzz);
  l_return.at(1, 2) = T(2) * (qyz + qwx);

  l_return.at(2, 0) = T(2) * (qxz + qwy);
  l_return.at(2, 1) = T(2) * (qyz - qwx);
  l_return.at(2, 2) = T(1) - T(2) * (qxx + qyy);
  return l_return;
};

template <typename T> mat<T, 4, 4> rotation(const quat<T> &p_quat) {
  return mat<T, 4, 4>::make(rotation_3(p_quat));
};

#if 0
template <typename T>
static mat<T, 4, 4> look_at(const vec<T, 3> &p_eye, const vec<T, 3> &p_left,
                            const vec<T, 3> &p_up, const vec<T, 3> &p_forward){

                            

    // Create a 4x4 orientation matrix from the right, up, and forward vectors
    // This is transposed which is equivalent to performing an inverse 
    // if the matrix is orthonormalized (in this case, it is).
    mat4 orientation = {
       vec4( xaxis.x, yaxis.x, zaxis.x, 0 ),
       vec4( xaxis.y, yaxis.y, zaxis.y, 0 ),
       vec4( xaxis.z, yaxis.z, zaxis.z, 0 ),
       vec4(   0,       0,       0,     1 )
    };
    
    // Create a 4x4 translation matrix.
    // The eye position is negated which is equivalent
    // to the inverse of the translation matrix. 
    // T(v)^-1 == T(-v)
    mat4 translation = {
        vec4(   1,      0,      0,   0 ),
        vec4(   0,      1,      0,   0 ), 
        vec4(   0,      0,      1,   0 ),
        vec4(-eye.x, -eye.y, -eye.z, 1 )
    };


};
#endif

template <typename T>
static mat<T, 4, 4> look_at(const vec<T, 3> &p_eye, const vec<T, 3> &p_center,
                            const vec<T, 3> &p_up) {
  assert_debug(p_eye != p_center);
  const vec<T, 3> f = normalize(p_center - p_eye);
  const vec<T, 3> s = normalize(cross(f, p_up));
  const vec<T, 3> u = cross(s, f);

  mat<T, 4, 4> l_result;
  l_result.at(0, 0) = s.x();
  l_result.at(1, 0) = s.y();
  l_result.at(2, 0) = s.z();
  l_result.at(0, 1) = u.x();
  l_result.at(1, 1) = u.y();
  l_result.at(2, 1) = u.z();
  l_result.at(0, 2) = -f.x();
  l_result.at(1, 2) = -f.y();
  l_result.at(2, 2) = -f.z();
  l_result.at(3, 0) = -dot(s, p_eye);
  l_result.at(3, 1) = -dot(u, p_eye);
  l_result.at(3, 2) = dot(f, p_eye);
  l_result.at(0, 3) = 0;
  l_result.at(1, 3) = 0;
  l_result.at(2, 3) = 0;
  l_result.at(3, 3) = 1;

  return l_result;
};

template <typename T>
static mat<T, 4, 4> perspective(T p_fovy, T p_aspect, T p_zNear, T p_zFar) {
  sys::sassert(m::abs(p_aspect - m::epsilon<T>::value()) > T(0));

  T const tanHalfFovy = m::tan(p_fovy / T(2));

  mat<T, 4, 4> l_result;
  l_result.at(0, 0) = T(1) / (p_aspect * tanHalfFovy);
  l_result.at(1, 1) = T(1) / (tanHalfFovy);
  l_result.at(2, 2) = -(p_zFar + p_zNear) / (p_zFar - p_zNear);
  l_result.at(2, 3) = -T(1);
  l_result.at(3, 2) = -(T(2) * p_zFar * p_zNear) / (p_zFar - p_zNear);

  l_result.at(0, 1) = 0;
  l_result.at(0, 2) = 0;
  l_result.at(0, 3) = 0;
  l_result.at(1, 0) = 0;
  l_result.at(1, 2) = 0;
  l_result.at(1, 3) = 0;
  l_result.at(2, 0) = 0;
  l_result.at(2, 1) = 0;
  l_result.at(3, 0) = 0;
  l_result.at(3, 1) = 0;
  l_result.at(3, 3) = 0;
  return l_result;
};

template <typename T>
static mat<T, 4, 4> orthographic(T p_left, T p_right, T p_bottom, T p_top,
                                 T p_zNear, T p_zFar) {
  mat<T, 4, 4> l_result;
  l_result.at(0, 0) = T(2) / (p_right - p_left);
  l_result.at(0, 1) = 0;
  l_result.at(0, 2) = 0;
  l_result.at(0, 3) = 0;

  l_result.at(1, 0) = 0;
  l_result.at(1, 1) = T(2) / (p_top - p_bottom);
  l_result.at(1, 2) = 0;
  l_result.at(1, 3) = 0;

  l_result.at(2, 0) = 0;
  l_result.at(2, 1) = 0;
  l_result.at(2, 2) = -T(2) / (p_zFar - p_zNear);
  l_result.at(2, 3) = 0;

  l_result.at(3, 0) = -(p_right + p_left) / (p_right - p_left);
  l_result.at(3, 1) = -(p_top + p_bottom) / (p_top - p_bottom);
  l_result.at(3, 2) = -(p_zFar + p_zNear) / (p_zFar - p_zNear);
  l_result.at(3, 3) = 1;
  return l_result;
};

template <typename T, typename AngleT>
static mat<T, 4, 4> rotate_around(const mat<T, 4, 4> &thiz, AngleT p_angle_rad,
                                  const vec<T, 3> &p_axis) {
  assert_debug(is_normalized(p_axis));

  AngleT c = m::cos(p_angle_rad);
  AngleT s = m::sin(p_angle_rad);
  normalize(p_axis);

  vec<T, 3> temp = (vec<T, 3>{1, 1, 1} - c) * p_axis;

  mat<T, 4, 4> l_rotate;
  l_rotate.at(0, 0) = c + temp.x() * p_axis.x();
  l_rotate.at(0, 1) = temp.x() * p_axis.y() + s * p_axis.z();
  l_rotate.at(0, 2) = temp.x() * p_axis.z() - s * p_axis.y();

  l_rotate.at(1, 0) = temp.y() * p_axis.x() - s * p_axis.z();
  l_rotate.at(1, 1) = c + temp.y() * p_axis.y();
  l_rotate.at(1, 2) = temp.y() * p_axis.z() + s * p_axis.x();

  l_rotate.at(2, 0) = temp.z() * p_axis.x() + s * p_axis.y();
  l_rotate.at(2, 1) = temp.z() * p_axis.y() - s * p_axis.x();
  l_rotate.at(2, 2) = c + temp.z() * p_axis.z();

  mat<T, 4, 4> l_result;
  l_result.col0() = thiz.col0() * l_rotate.at(0, 0) +
                    thiz.col1() * l_rotate.at(0, 1) +
                    thiz.col2() * l_rotate.at(0, 2);
  l_result.col1() = thiz.col0() * l_rotate.at(1, 0) +
                    thiz.col1() * l_rotate.at(1, 1) +
                    thiz.col2() * l_rotate.at(1, 2);
  l_result.col2() = thiz.col0() * l_rotate.at(2, 0) +
                    thiz.col1() * l_rotate.at(2, 1) +
                    thiz.col2() * l_rotate.at(2, 2);
  l_result.col3() = thiz.col3();
  return l_result;
};

template <typename T>
quat<T> rotate_around(T p_angle, const vec<T, 3> &p_axis) {
  T a = p_angle;
  T s = m::sin(a * T(0.5f));
  return quat<T>::make_w_xyz(m::cos(a * T(0.5)), p_axis * s);
};

template <typename T>
static mat<T, 4, 4> translate(const vec<T, 3> &p_translate) {
  mat<T, 4, 4> l_mat = l_mat.getIdentity();
  l_mat.col3().x() = p_translate.x();
  l_mat.col3().y() = p_translate.y();
  l_mat.col3().z() = p_translate.z();
  return l_mat;
};

} // namespace m