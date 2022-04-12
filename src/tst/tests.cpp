#include <bgfx/bgfx.h>
#include <cor/cor.hpp>
#include <sys/sys.hpp>

int main() {
  boost::container::vector<int> zd;

  auto l_frame_buffer =
      bgfx::createFrameBuffer(100, 100, bgfx::TextureFormat::RG8);
  bgfx::setViewRect(0, 0, 0, 100, 100);
  bgfx::setViewFrameBuffer(0, l_frame_buffer);
  // bgfx::setUniform(UniformHandle _handle, const void *_value);
  // bgfx::setView
  // bgfx::submit()
  /*
  bgfx::TextureHandle l_handle =
      bgfx::createTexture2D(100, 100, false, 0, bgfx::TextureFormat::RG8);
      */
};

#include <rast/rast.hpp>