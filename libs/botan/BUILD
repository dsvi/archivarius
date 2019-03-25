# get the files through ./configure.py --amalgamation --minimized-build --single-amalgamation-file --enable-modules=chacha20poly1305,chacha_avx2,blake2,system_rng
cc_library(
    name = "botan",
    srcs = [
        "botan_all.cpp",
        "botan_all.h",
        "botan_all_internal.h"
    ],
    includes=["."],
    visibility = ["//visibility:public"]
)
