project(MyLittleEngine)
cmake_minimum_required(VERSION 3.16)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

# set(ENABLE_SAFETY_CHECKS true)
# set(ENABLE_ADDRESS true)
# set(ENABLE_UNDEFINED true)
# set(WIN_HEADLESS true)
# set(PLATFORM_WEBASSEMBLY true)

if(PLATFORM_WEBASSEMBLY)
    set(CMAKE_C_COMPILER "emcc")
    set(CMAKE_CXX_COMPILER "emcc")
    add_link_options(--bind)
    add_link_options(-sALLOW_MEMORY_GROWTH)
    add_compile_definitions(PLATFORM_WEBASSEMBLY_PREPROCESS=1)
else()
    add_compile_definitions(PLATFORM_WEBASSEMBLY_PREPROCESS=0)
endif()

if(WIN_HEADLESS)
    add_compile_definitions(WIN_HEADLESS_PREPROCESS=1)
else()
    add_compile_definitions(WIN_HEADLESS_PREPROCESS=0)
endif()

if(ENABLE_SAFETY_CHECKS)
    add_compile_definitions(DEBUG_PREPROCESS=1)
else()
    add_compile_definitions(DEBUG_PREPROCESS=0)
endif()

if(ENABLE_ADDRESS)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if(ENABLE_UNDEFINED)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

add_library(BGFX_API INTERFACE)
target_include_directories(BGFX_API INTERFACE ./tp/bgfx_api/)

add_library(EMSCRIPTEN_API INTERFACE)
target_include_directories(EMSCRIPTEN_API INTERFACE ./tp/emscripten/)

add_library(STB INTERFACE)
target_include_directories(STB INTERFACE ./tp/stb/)

add_library(DOCTEST INTERFACE)
target_include_directories(DOCTEST INTERFACE ./tp/doctest/)

add_library(WholeProject INTERFACE)
target_include_directories(WholeProject INTERFACE ./src/)
target_link_libraries(WholeProject INTERFACE BGFX_API)


add_library(WIN INTERFACE)

if (WIN_HEADLESS)
elseif(PLATFORM_WEBASSEMBLY)
    target_link_libraries(WIN INTERFACE EMSCRIPTEN_API)
else()
    find_package(X11 REQUIRED)
    target_link_libraries(WIN INTERFACE ${X11_LIBRARIES})
endif()

target_link_libraries(WIN INTERFACE WholeProject)

add_executable(TESTS ./src/tst/tests.cpp ./src/tst/test_assets.cpp ./src/tst/test_numbers.cpp ./src/tst/test_rasterizer_assets.cpp ./src/tst/test_rasterizer.cpp ./src/tst/test_window.cpp)
target_link_libraries(TESTS PUBLIC WIN)
target_link_libraries(TESTS PUBLIC STB)
target_link_libraries(TESTS PUBLIC DOCTEST)

add_library(API SHARED ./src/api/c_api.cpp)
target_link_libraries(API PUBLIC WIN)

add_subdirectory(./sandbox)