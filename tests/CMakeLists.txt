include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
)
FetchContent_MakeAvailable(unity)

add_executable(test_ringbuf test_ringbuf.c)
target_link_libraries(test_ringbuf PRIVATE core unity)
target_compile_options(test_ringbuf PRIVATE -Wall -Wextra)
add_test(NAME ringbuf COMMAND test_ringbuf)


add_executable(test_cmd_parser test_cmd_parser.c)
target_link_libraries(test_cmd_parser PRIVATE core unity)
target_compile_options(test_cmd_parser PRIVATE -Wall -Wextra)
add_test(NAME cmd_parser COMMAND test_cmd_parser)
