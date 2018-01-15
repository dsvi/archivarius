project "zstd"
  language    "C"
  kind        "StaticLib"
  --warnings    "off"

files
{
"common/*.c",
"compress/*.c",
"decompress/*.c",
"dictBuilder/*.c",
}

includedirs {
  "common",
  ".",
}




