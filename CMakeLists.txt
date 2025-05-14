cmake_minimum_required(VERSION 3.20)
project(ita LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ita
    src/main.cpp
)

target_include_directories(ita PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_options(ita PRIVATE -Wall -Wextra -pedantic)
