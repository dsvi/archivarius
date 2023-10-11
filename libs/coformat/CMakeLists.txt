cmake_minimum_required(VERSION 3.11)

project(coformat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(coformat
    coformat.h
    coformat.c++)
target_include_directories(coformat PUBLIC .)

add_executable(coformat-example main.c++)
target_link_libraries(coformat-example PRIVATE coformat)
