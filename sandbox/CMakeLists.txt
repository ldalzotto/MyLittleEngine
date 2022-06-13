

add_executable(SANDBOX_MESH_VIZUALIZER ./mesh_visualizer.cpp)
target_link_libraries(SANDBOX_MESH_VIZUALIZER PUBLIC ENGINE)
target_include_directories(SANDBOX_MESH_VIZUALIZER PUBLIC ./src/api/)
