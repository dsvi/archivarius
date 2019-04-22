cmake_minimum_required(VERSION 3.14)

project(botan  
    VERSION 1.3.8
    DESCRIPTION "zstd")


add_library(zstd  
./common/entropy_common.c
./common/fse_decompress.c
./common/threading.c
./common/pool.c
./common/zstd_common.c
./common/error_private.c
./common/xxhash.c
./common/debug.h
./common/pool.h
./common/threading.h
./common/bitstream.h
./common/error_private.h
./common/zstd_errors.h
./common/fse.h
./common/huf.h
./common/mem.h
./common/zstd_internal.h
./compress/hist.c
./compress/fse_compress.c
./compress/huf_compress.c
./compress/zstd_compress.c
./compress/zstd_fast.c
./compress/zstd_double_fast.c
./compress/zstd_lazy.c
./compress/zstd_opt.c
./compress/zstd_ldm.c
./compress/hist.h
./compress/zstd_compress_internal.h
./compress/zstd_fast.h
./compress/zstd_double_fast.h
./compress/zstd_lazy.h
./compress/zstd_opt.h
./compress/zstd_ldm.h
./compress/zstdmt_compress.h
./decompress/huf_decompress.c
./decompress/zstd_decompress.c
./decompress/zstd_decompress_block.c
./decompress/zstd_ddict.c
./decompress/zstd_decompress_internal.h
./decompress/zstd_decompress_block.h
./decompress/zstd_ddict.h
./zstd.h )

target_include_directories(zstd PUBLIC . ./common)


