

add_executable(TEST_0 ./test_0.cpp)
target_link_libraries(TEST_0 PUBLIC API)
target_include_directories(TEST_0 PUBLIC ./src/api/)
if(PLATFORM_WEBASSEMBLY)
    target_link_options(TEST_0 PRIVATE --bind)
    # target_link_options(TEST_0 PRIVATE -sASYNCIFY)
    target_link_options(TEST_0 PRIVATE -sALLOW_MEMORY_GROWTH)
endif()
