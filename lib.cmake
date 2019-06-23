add_subdirectory("libs/botan")
add_subdirectory("libs/zstd")
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
add_subdirectory("libs/range-v3")
add_subdirectory("libs/protobuf/cmake")
add_subdirectory("libs/fmt")

set_target_properties(libprotobuf-lite PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

set_target_properties(fmt PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

set(archivarius-lib-srcs 
./archivarius/archivarius.h
./archivarius/archive.c++
./archivarius/archive.h
./archivarius/buffer.c++
./archivarius/buffer.h
./archivarius/catalogue.c++
./archivarius/catalogue.h
./archivarius/checksum.h
./archivarius/checksumer.c++
./archivarius/checksumer.h
./archivarius/encryption_params.c++
./archivarius/encryption_params.h
./archivarius/exception.c++
./archivarius/exception.h
./archivarius/file_content_creator.c++
./archivarius/file_content_creator.h
./archivarius/file_content_ref.h
./archivarius/filesystem_state.c++
./archivarius/filesystem_state.h
./archivarius/filters.c++
./archivarius/filters.h
./archivarius/format.pb.cc
./archivarius/format.pb.h
./archivarius/format.proto
./archivarius/globals.c++
./archivarius/globals.h
./archivarius/piping.c++
./archivarius/piping.h
./archivarius/piping_blake2b.c++
./archivarius/piping_blake2b.h
./archivarius/piping_chacha.c++
./archivarius/piping_chacha.h
./archivarius/piping_chapoly.c++
./archivarius/piping_chapoly.h
./archivarius/piping_xxhash.c++
./archivarius/piping_xxhash.h
./archivarius/piping_zstd.c++
./archivarius/piping_zstd.h
./archivarius/platform.c++
./archivarius/platform.h
./archivarius/precomp.c++
./archivarius/precomp.h
./archivarius/restore.c++
./archivarius/restore.h
./archivarius/stream.c++
./archivarius/stream.h
./archivarius/xxhash.h
)

add_library(archivarius-lib ${archivarius-lib-srcs})
target_include_directories(archivarius-lib PUBLIC .)
set_target_properties(archivarius-lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(archivarius-lib PUBLIC botan fmt zstd range-v3 libprotobuf-lite acl stdc++fs)
