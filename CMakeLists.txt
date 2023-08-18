cmake_minimum_required(VERSION 3.14)

include(FetchContent)
 
# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_POLICY_DEFAULT_CMP0069 NEW) # enable LTO on all compilers
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

option(ARCHIVARIUS_STATIC_BUILD "Build static version of the program, without dependency on any shared library." ON)

if (ARCHIVARIUS_STATIC_BUILD)
    #SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

project(archivarius
    VERSION 0.0.10
    DESCRIPTION "Archivarius lib and CLI tool")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Using libc++")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
    string(APPEND CMAKE_CXX_FLAGS " -Wno-unqualified-std-cast-call")
    # -lc++abi is required for static build to work
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -fuse-ld=lld -lc++abi")
    #string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
endif()

# otherwise cmake --install tries to install all the dependeencies
install(CODE "set(CMAKE_INSTALL_LOCAL_ONLY true)")

add_subdirectory("libs/botan")
add_subdirectory("libs/protobuf/cmake")

FetchContent_Declare(
    fmt
    URL https://github.com/fmtlib/fmt/archive/refs/tags/10.0.0.tar.gz
    URL_HASH SHA256=ede1b6b42188163a3f2e0f25ad5c0637eca564bd8df74d02e31a311dd6b37ad8
)
FetchContent_MakeAvailable(fmt)

if (ARCHIVARIUS_STATIC_BUILD)
    set(ZSTD_LEGACY_SUPPORT OFF)
    set(ZSTD_MULTITHREAD_SUPPORT OFF)
    FetchContent_Declare(
        zstd
        URL https://github.com/facebook/zstd/releases/download/v1.5.5/zstd-1.5.5.tar.zst
        SOURCE_SUBDIR build/cmake
    )
    FetchContent_MakeAvailable(zstd)
    target_include_directories(libzstd_static PUBLIC $<BUILD_INTERFACE:${zstd_SOURCE_DIR}/lib>)
    add_library(zstd ALIAS libzstd_static)
endif()

set(archivarius-src
archivarius/archivarius.h
archivarius/archive.c++
archivarius/archive.h
archivarius/buffer.c++
archivarius/buffer.h
archivarius/catalogue.c++
archivarius/catalogue.h
archivarius/cfmt.h
archivarius/checksum.h
archivarius/checksumer.h
archivarius/checksumer_blake2b.c++
archivarius/checksumer_blake2b.h
archivarius/checksumer_xxhash.c++
archivarius/checksumer_xxhash.h
archivarius/cmd_line_parser.c++
archivarius/cmd_line_parser.h
archivarius/config.c++
archivarius/config.h
archivarius/encryption_params.c++
archivarius/encryption_params.h
archivarius/exception.c++
archivarius/exception.h
archivarius/file_content_creator.c++
archivarius/file_content_creator.h
archivarius/file_content_ref.h
archivarius/filesystem_state.c++
archivarius/filesystem_state.h
archivarius/filters.c++
archivarius/filters.h
archivarius/format.pb.cc
archivarius/format.pb.h
archivarius/format.proto
archivarius/globals.c++
archivarius/globals.h
archivarius/main.c++
archivarius/piping.c++
archivarius/piping.h
archivarius/piping_chacha.c++
archivarius/piping_chacha.h
archivarius/piping_chapoly.c++
archivarius/piping_chapoly.h
archivarius/piping_csum.c++
archivarius/piping_csum.h
archivarius/piping_zstd.c++
archivarius/piping_zstd.h
archivarius/platform.c++
archivarius/platform.h
archivarius/precomp.c++
archivarius/precomp.h
archivarius/property_tree.c++
archivarius/property_tree.h
archivarius/pump.c++
archivarius/pump.h
archivarius/restore.c++
archivarius/restore.h
archivarius/stream.c++
archivarius/stream.h
archivarius/test.c++
archivarius/test.h
archivarius/testing.c++
archivarius/testing.h
archivarius/version.h
archivarius/xxhash.h
)

configure_file(${PROJECT_SOURCE_DIR}/archivarius/version.h.in ${PROJECT_SOURCE_DIR}/archivarius/version.h)

add_executable(archivarius ${archivarius-src})
target_link_libraries(archivarius PRIVATE botan fmt::fmt zstd libprotobuf-lite acl)

install(TARGETS ${PROJECT_NAME})

set(CPACK_BINARY_DEB "ON")
set(CPACK_BINARY_TXZ "ON")
set(CPACK_BINARY_STGZ "OFF")
set(CPACK_BINARY_TGZ "OFF")
set(CPACK_BINARY_TZ "OFF")

set(CPACK_CMAKE_GENERATOR Ninja)
# DEB only
set(CPACK_PACKAGE_CONTACT https://github.com/dsvi/archivarius)
set(CPACK_DEBIAN_COMPRESSION_TYPE lzma)

include(CPack)
