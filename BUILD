cc_binary(
    name = "archivarius",
    srcs = glob(["src/*.c++"]) + glob(["src/*.cc"]) + glob(["src/*.h"]),
    deps = [
        "//libs/botan:botan",
        "//libs/zstd:zstd",
        "//libs/fmt:fmt",
        "//libs/range-v3:range",
        "//libs/protobuf:protobuf_lite",
    ],
    linkopts = ["-lstdc++fs -lacl"],
    defines = select({
        ":debug": ["DEBUG"],
        "//conditions:default": []
    }),
)

config_setting(
    name = "debug",
    values = { "compilation_mode": "dbg" }
)

