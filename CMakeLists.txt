project(MyLittleEngine)
cmake_minimum_required(VERSION 3.16)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

# set(ENABLE_SAFETY_CHECKS true)
# set(ENABLE_ADDRESS true)
# set(ENABLE_UNDEFINED true)

add_compile_options(-fPIC)
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

add_library(WholeProject INTERFACE)
target_include_directories(WholeProject INTERFACE ./src/)
target_link_libraries(WholeProject INTERFACE BGFX_API)

add_library(SYS STATIC ./src/sys/sys.cpp)
target_link_libraries(SYS PUBLIC WholeProject)

find_package(X11 REQUIRED)

add_library(WIN STATIC ./src/sys/win.cpp)
target_link_libraries(WIN PUBLIC WholeProject)
target_link_libraries(WIN PUBLIC SYS)
target_link_libraries(WIN PUBLIC ${X11_LIBRARIES})

add_library(API SHARED ./src/eng/api.cpp)
target_link_libraries(API PUBLIC SYS)
target_link_libraries(API PUBLIC WIN)

add_executable(TESTS ./src/tst/tests.cpp)
target_link_libraries(TESTS PUBLIC SYS)
target_link_libraries(TESTS PUBLIC WIN)
