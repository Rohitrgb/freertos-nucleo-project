add_library(core STATIC ringbuf.c cmd_parser.c debounce.c)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(core PRIVATE -Wall -Wextra -Wconversion -Wpedantic)
