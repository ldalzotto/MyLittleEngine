#pragma once

#include <doctest.h>
#include <eng/engine.hpp>

#if !RUNTIME_CI_PREPROCESS

#define TEST_RESOURCE_PATH_RAW_PREPROCESS                                      \
  "/media/loic/SSD/SoftwareProjects/Once/test_data/"

#define WRITE_OUTPUT_TO_TMP 0
#define WRITE_OUTPUT_TO_RESULT 0

#else

#define WRITE_OUTPUT_TO_TMP 0
#define WRITE_OUTPUT_TO_RESULT 0

#endif

inline static constexpr auto TEST_RESOURCE_PATH =
    container::arr_literal<ui8>(TEST_RESOURCE_PATH_RAW_PREPROCESS);

struct TestImageAssertionConfig {
  ui8 m_write_output_to_tmp;
  ui8 m_write_output_to_result;
  container::range<ui8> m_tmp_folder;
  container::range<ui8> m_result_folder;

  inline static constexpr TestImageAssertionConfig
  make(const container::range<ui8> &p_tmp_folder,
       const container::range<ui8> &p_result_folder) {
    return TestImageAssertionConfig{
        .m_write_output_to_tmp = WRITE_OUTPUT_TO_TMP,
        .m_write_output_to_result = WRITE_OUTPUT_TO_RESULT,
        .m_tmp_folder = {.m_begin = (ui8 *)p_tmp_folder.m_begin,
                         .m_count = p_tmp_folder.m_count},
        .m_result_folder = {
            .m_begin = (ui8 *)p_result_folder.m_begin,
            .m_count = p_result_folder.m_count,
        }};
  };
};

struct TestUtils {
  static i32 write_png(const ui8 *filename, i32 x, i32 y, i32 comp,
                       const void *data, i32 stride_bytes);
  static container::range<ui8>
  write_png_to_mem(const ui8 *pixels, i32 stride_bytes, i32 x, i32 y, i32 n);
  static void write_free(ui8 *p_ptr);
  static ui8 *read_png(const ui8 *filename, i32 *x, i32 *y, i32 *comp,
                       i32 req_comp);
  static void read_free(ui8 *ptr);

  inline static container::span<ui8>
  file_path_null_terminated(const container::range<ui8> &p_left,
                            const container::range<ui8> &p_relative_path) {
    container::span<ui8> l_tmp_path_null_terminated;
    l_tmp_path_null_terminated.allocate(p_left.count() +
                                        p_relative_path.count() + 1);
    l_tmp_path_null_terminated.range().copy_from(p_left);
    l_tmp_path_null_terminated.range()
        .slide(p_left.size_of())
        .copy_from(p_relative_path);
    l_tmp_path_null_terminated.at(l_tmp_path_null_terminated.count() - 1) =
        '\0';
    return l_tmp_path_null_terminated;
  };

  template <typename Engine>
  inline static void
  assert_frame_equals(const container::range<ui8> &p_relative_path,
                      eng::engine_api<Engine> p_engine, ui16 p_width,
                      ui16 p_height, const TestImageAssertionConfig &p_config) {
    container::range<ui8> l_png_frame;
    container::span<rgb_t> l_frame_buffer_rgb;
    {

      container::range<rgba_t> p_frame_buffer_rgba =
          p_engine.window_system()
              .window_get_image_buffer(p_engine.thiz.m_window)
              .m_data.range()
              .template cast_to<rgba_t>();

      l_frame_buffer_rgb.allocate(p_frame_buffer_rgba.count());
      for (auto i = 0; i < p_frame_buffer_rgba.count(); ++i) {
        l_frame_buffer_rgb.at(i) = p_frame_buffer_rgba.at(i).xyz();
      }

      if (p_config.m_write_output_to_tmp) {
        container::span<ui8> l_tmp_path_null_terminated =
            TestUtils::file_path_null_terminated(p_config.m_tmp_folder,
                                                 p_relative_path);
        TestUtils::write_png((const ui8 *)l_tmp_path_null_terminated.data(),
                             p_width, p_height, 3,
                             (ui8 *)l_frame_buffer_rgb.data(), 3 * p_width);
        l_tmp_path_null_terminated.free();
      }

      if (p_config.m_write_output_to_result) {
        container::span<ui8> l_tmp_path_null_terminated =
            TestUtils::file_path_null_terminated(p_config.m_result_folder,
                                                 p_relative_path);
        TestUtils::write_png((const ui8 *)l_tmp_path_null_terminated.data(),
                             p_width, p_height, 3,
                             (ui8 *)l_frame_buffer_rgb.data(), 3 * p_width);
        l_tmp_path_null_terminated.free();
      }

      l_png_frame =
          TestUtils::write_png_to_mem((const ui8 *)l_frame_buffer_rgb.data(),
                                      3 * p_width, p_width, p_height, 3);
    }

    if (!p_config.m_write_output_to_tmp) {

      container::span<ui8> l_expected_path_null_terminated =
          TestUtils::file_path_null_terminated(p_config.m_result_folder,
                                               p_relative_path);
      i32 l_width, l_height, l_channel;
      container::range<ui8> l_expected_frame;
      l_expected_frame.m_begin = TestUtils::read_png(
          (const ui8 *)l_expected_path_null_terminated.data(), &l_width,
          &l_height, &l_channel, 0);
      l_expected_frame.m_count = l_width * l_height * l_channel;
      l_expected_path_null_terminated.free();

      REQUIRE(l_expected_frame.is_contained_by(
          l_frame_buffer_rgb.range().cast_to<ui8>()));

      TestUtils::read_free(l_expected_frame.m_begin);
    }
    TestUtils::write_free(l_png_frame.m_begin);

    l_frame_buffer_rgb.free();
  };
};