cmake_minimum_required(VERSION 3.22)

include(FetchContent)
include(cmake/AddFromArchive.cmake)
include(cmake/CPM.cmake)

# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    string(APPEND CMAKE_CXX_FLAGS " -flto=full")
    string(APPEND CMAKE_C_FLAGS   " -flto=full")
endif()

option(ARCHIVARIUS_STATIC_BUILD "Build static version of the program, without dependency on any shared library." ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

project(archivarius
    VERSION 1.1.6
    DESCRIPTION "Archivarius CLI tool")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Using libc++")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
    string(APPEND CMAKE_CXX_FLAGS " -Wno-unqualified-std-cast-call")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -fuse-ld=lld")
    # With -static the driver omits -lc++abi (all C++ runtime symbols such
    # as __cxa_guard_* and operator new come from there), so add it explicitly.
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -lc++abi")
    #string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
endif()

set(BOTAN_ARCHIVE botan-2.15.0.tar.gz)
set(BOTAN_MODULES_LIST
    chacha20poly1305
    blake2
    system_rng
)
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    list(APPEND BOTAN_MODULES_LIST chacha_avx2)
endif()
add_subdirectory(libs/botan EXCLUDE_FROM_ALL)
add_from_archive(libs/protobuf-3.20.2.7z cmake)
CPMAddPackage("gh:dsvi/coformat#main")
CPMAddPackage("gh:dsvi/progress-bar#main")
set(ZSTD_LEGACY_SUPPORT OFF)
set(ZSTD_MULTITHREAD_SUPPORT OFF)
set(ZSTD_BUILD_DICTBUILDER OFF)
add_from_archive_url(https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.zst build/cmake)

if (ARCHIVARIUS_STATIC_BUILD)
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -static")
    # under -static the linker only searches for static archives
    find_library(ACL_LIBRARY NAMES libacl.a)
else()
    find_library(ACL_LIBRARY NAMES acl)
endif()

if(NOT ACL_LIBRARY)
    message(FATAL_ERROR "libacl not found")
endif()

set(archivarius-src

src/property_tree/file_reader.c++
src/property_tree/file_reader.h
src/property_tree/property_tree.c++
src/property_tree/property_tree.h

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
src/time.c++
src/time.h
src/version.h
src/xxhash.h
)

configure_file(${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_SOURCE_DIR}/src/version.h)

# Common dependencies of both executables.
set(archivarius-deps
    botan
    coformat
    progress-bar
    libzstd_static
    libprotobuf-lite
    ${ACL_LIBRARY}
)

# Create an archivarius executable with the given name.
# Any extra arguments (e.g. EXCLUDE_FROM_ALL) are passed through to add_executable().
function(add_archivarius_tool name)
    add_executable(${name} ${ARGN} ${archivarius-src})
    target_precompile_headers(${name} PRIVATE src/precomp.h)
    target_link_libraries(${name} PRIVATE ${archivarius-deps})
endfunction()

add_archivarius_tool(archivarius)
# A build of the tool with the self-test compiled in (TEST define enables
# test() from testing.c++ in main()). Excluded from the default build;
# build it explicitly with: cmake --build . --target archivarius-test
add_archivarius_tool(archivarius-test EXCLUDE_FROM_ALL)
target_compile_definitions(archivarius-test PRIVATE TEST)
