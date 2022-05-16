## Welcome
This project emulates computations done by the GPU hardware to draw 3D objects.
The goal of this project is to remove the usage of graphics API like OpenGL or Vulkan by re-implementing rasterization functions.

This project was made to challenge myself to write a software rasterizer from scratch. Learning graphics concepts and maths in the process.
It is a learning journey, so as much as possible, it will use handmade solutions (memory containers, maths library, 3D scene management, 3D graphics abstraction, OS interactions).

## Third party
Usage of third party libraries is limited to :

* stbimage for png loading and write.
* emscripten headers to build to webassembly
