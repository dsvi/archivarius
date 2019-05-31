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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message("Using libc++")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -stdlib=libc++ -lc++abi")
endif()


#string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address -fsanitize-recover=address")

project(archivarius  
    VERSION 0.0 
    DESCRIPTION "Archivarius CLI tool")

add_subdirectory("libs/botan")
add_subdirectory("libs/zstd")
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
add_subdirectory("libs/range-v3")
add_subdirectory("libs/protobuf/cmake")
add_subdirectory("libs/fmt")

set(archivarius_srcs 
./src/zstd_filter.h
./src/format.proto
./src/format.pb.h
./src/format.pb.cc
./src/precomp.c++
./src/zstd_filter.c++
./src/exception.c++
./src/xxhash.h
./src/exception.h
./src/buffer.h
./src/buffer.c++
./src/piping.c++
./src/stream.h
./src/piping.h
./src/piping_zstd.c++
./src/piping_zstd.h
./src/encryption_params.h
./src/piping_chapoly.h
./src/piping_chacha.c++
./src/piping_chacha.h
./src/checksumer.c++
./src/piping_xxhash.h
./src/piping_blake2b.h
./src/piping_blake2b.c++
./src/piping_xxhash.c++
./src/stream.c++
./src/checksumer.h
./src/file_content_ref.h
./src/checksum.h
./src/encryption_params.c++
./src/property_tree.c++
./src/property_tree.h
./src/platform.h
./src/platform.c++
./src/test.h
./src/archive.h
./src/cmd_line_parser.c++
./src/cmd_line_parser.h
./src/config.h
./src/piping_chapoly.c++
./src/precomp.h
./src/catalogue.h
./src/file_content_creator.h
./src/file_content_creator.c++
./src/globals.h
./src/globals.c++
./src/filesystem_state.h
./src/config.c++
./src/archive.c++
./src/filesystem_state.c++
./src/catalogue.c++
./src/restore.h
./src/filters.h
./src/filters.c++
./src/restore.c++
./src/main.c++
./src/test.c++
)

add_executable(archivarius ${archivarius_srcs})
target_include_directories(archivarius PRIVATE src)

set_target_properties(archivarius PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_link_libraries(archivarius PUBLIC botan fmt zstd range-v3 libprotobuf-lite acl stdc++fs)

