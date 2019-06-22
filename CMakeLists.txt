 cmake_minimum_required(VERSION 3.14)
 
# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build mode")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message("Using libc++")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -Wl,-Bstatic -lc++ -Wl,-Bdynamic")
endif()

project(archivarius  
    VERSION 0.0 
    DESCRIPTION "Archivarius lib and CLI tool")


include(console-tool.cmake)
