 cmake_minimum_required(VERSION 3.14)
 
# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

option(ARCHIVARIUS_STATIC_BUILD "Build static version of the program, without dependency on any shared library." OFF)

if (ARCHIVARIUS_STATIC_BUILD)
    #SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

project(archivarius  
    VERSION 0.0.8
    DESCRIPTION "Archivarius lib and CLI tool")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Using libc++")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
    # -lc++abi is required for static build to work
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -fuse-ld=lld -lc++abi")
    #string(APPEND CMAKE_CXX_FLAGS_DEBUG " -O1 -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
endif()

include(console-tool.cmake)
