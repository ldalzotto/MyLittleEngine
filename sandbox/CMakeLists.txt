

add_executable(TEST_0 ./test_0.cpp)
target_link_libraries(TEST_0 PUBLIC API)
target_include_directories(TEST_0 PUBLIC ./src/api/)