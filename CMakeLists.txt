project(MyLittleEngine)
cmake_minimum_required(VERSION 3.16)
enable_language(CXX)

add_library(BOOST INTERFACE)
target_include_directories(BOOST INTERFACE ./tp/boost_1_78_0/)

add_library(Eigen INTERFACE)
target_include_directories(Eigen INTERFACE ./tp/eigen-3.4.0/)

add_library(BGFX_API INTERFACE)
target_include_directories(BGFX_API INTERFACE ./tp/bgfx_api/)

add_library(ThirdParties INTERFACE)
target_link_libraries(ThirdParties INTERFACE BOOST)
target_link_libraries(ThirdParties INTERFACE Eigen)
target_link_libraries(ThirdParties INTERFACE BGFX_API)

add_library(WholeProject INTERFACE)
target_link_libraries(WholeProject INTERFACE ThirdParties)
target_include_directories(WholeProject INTERFACE ./src/)


add_library(SYS STATIC ./src/sys/sys.cpp)
target_link_libraries(SYS PUBLIC WholeProject)

add_executable(TESTS ./src/tst/tests.cpp)
target_link_libraries(TESTS PUBLIC SYS)
