cmake_minimum_required(VERSION 3.22)
project(typtr)

add_executable(
  typtr
  main.c wordlist.c term_handler.c text.c stats.c file_util.c keys.c
)

target_compile_options(
  typtr PUBLIC
  # "-Weverything"
  # "-Werror"
  "-Wall" "-Wpedantic" "-Wextra"
  "-Wsign-conversion" "-Wdocumentation-unknown-command" "-Wmissing-prototypes"
  "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb;-fsanitize=memory;-fsanitize=undefined>"
)
target_link_options(
  typtr PUBLIC
  "$<$<CONFIG:DEBUG>:-fsanitize=memory;-fsanitize=undefined>"
)

add_executable(dconv dconv.c stats.c keys.c)

target_compile_options(
  dconv PUBLIC
  # "-Weverything"
  # "-Werror"
  "-Wall" "-Wpedantic" "-Wextra"
  "-Wsign-conversion" "-Wdocumentation-unknown-command" "-Wmissing-prototypes"
  "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb;-fsanitize=memory;-fsanitize=undefined>"
)

target_link_options(
  dconv PUBLIC
  "$<$<CONFIG:DEBUG>:-fsanitize=memory;-fsanitize=undefined>"
)
