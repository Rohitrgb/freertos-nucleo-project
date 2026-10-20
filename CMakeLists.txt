cmake_minimum_required(VERSION 3.20)
project(freertos_nucleo_project C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(core)
add_subdirectory(tests)
