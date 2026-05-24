cmake_minimum_required(VERSION 3.10)
project(bimbim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories("D:/glfw-3.4.bin.WIN64/include")
link_directories("D:/glfw-3.4.bin.WIN64/lib-mingw-w64")

# add_executable(main main.c)
# add_executable(main 3D.c)
# add_executable(main 2D_fast.c)
add_executable(main 2D_fast_with_cols.c)

target_link_libraries(main glfw3 opengl32)