cmake_minimum_required(VERSION 3.10)
project(typtr)

add_executable(typtr main.c)
add_compile_options(
  "-Wall" "-Wpedantic" "-Wextra" "-Werror"
  "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
)
