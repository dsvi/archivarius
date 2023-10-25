cmake_minimum_required(VERSION 3.22)

include(FetchContent)
include(cmake/AddFromArchive.cmake)

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
    VERSION 1.1.1
    DESCRIPTION "Archivarius lib and CLI tool")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Using libc++")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
    string(APPEND CMAKE_CXX_FLAGS " -Wno-unqualified-std-cast-call")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -fuse-ld=lld")
    #string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
endif()

# otherwise cmake --install tries to install all the dependeencies
install(CODE "set(CMAKE_INSTALL_LOCAL_ONLY true)")

set(BOTAN_ARCHIVE botan-2.15.0.tar.gz)
set(BOTAN_MODULES_LIST
    chacha20poly1305
    blake2
    system_rng
)
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    list(APPEND BOTAN_MODULES_LIST chacha_avx2)
endif()
add_subdirectory(libs/botan)
add_from_archive(libs/protobuf-3.20.2.7z cmake)
add_subdirectory(libs/coformat)
add_subdirectory(libs/progress-bar)

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
    set(ZSTD_LIB_NAME libzstd_static)
else()
    set(ZSTD_LIB_NAME zstd)
endif()

set(archivarius-src
src/archive.c++
src/archive.h
src/buffer.c++
src/buffer.h
src/catalogue.c++
src/catalogue.h
src/checksum.h
src/checksumer.h
src/checksumer_blake2b.c++
src/checksumer_blake2b.h
src/checksumer_xxhash.c++
src/checksumer_xxhash.h
src/cmd_line_parser.c++
src/cmd_line_parser.h
src/config.c++
src/config.h
src/encryption_params.c++
src/encryption_params.h
src/exception.c++
src/exception.h
src/file_content_creator.c++
src/file_content_creator.h
src/file_content_ref.h
src/filesystem_state.c++
src/filesystem_state.h
src/filters.c++
src/filters.h
src/format.pb.cc
src/format.pb.h
src/format.proto
src/globals.c++
src/globals.h
src/main.c++
src/piping.c++
src/piping.h
src/piping_chacha.c++
src/piping_chacha.h
src/piping_chapoly.c++
src/piping_chapoly.h
src/piping_csum.c++
src/piping_csum.h
src/piping_zstd.c++
src/piping_zstd.h
src/platform.c++
src/platform.h
src/precomp.c++
src/precomp.h
src/property_tree.c++
src/property_tree.h
src/pump.c++
src/pump.h
src/restore.c++
src/restore.h
src/stream.c++
src/stream.h
src/test.c++
src/test.h
src/testing.c++
src/testing.h
src/version.h
src/xxhash.h
)

configure_file(${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_SOURCE_DIR}/src/version.h)

add_executable(archivarius ${archivarius-src})
target_precompile_headers(archivarius PRIVATE src/precomp.h)
target_link_libraries(archivarius PRIVATE botan coformat progress-bar ${ZSTD_LIB_NAME} libprotobuf-lite acl)

install(TARGETS archivarius)

set(CPACK_BINARY_DEB "ON")
set(CPACK_BINARY_TXZ "ON")
set(CPACK_BINARY_STGZ "OFF")
set(CPACK_BINARY_TGZ "OFF")
set(CPACK_BINARY_TZ "OFF")

set(CPACK_CMAKE_GENERATOR Ninja)
# DEB only
set(CPACK_PACKAGE_CONTACT https://gitlab.com/44100Hz/archivarius)
set(CPACK_DEBIAN_COMPRESSION_TYPE lzma)

include(CPack)
