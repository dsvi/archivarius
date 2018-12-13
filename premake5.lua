workspace "archivarius"
location ("build")
configurations { "release", "debug" }

newaction {
   trigger     = "clean",
   description = "cleans all the generated files",
   execute     = function ()
      os.rmdir("./build")
      print("done.")
   end
}

path.iscppfile = function (fname)
  return path.hasextension(fname, { ".cc", ".cpp", ".cxx", ".c++" })
end

--flags { "C++14", "StaticRuntime", "NoBufferSecurityCheck", "LinkTimeOptimization" }
--flags { "C++14", "StaticRuntime", "NoBufferSecurityCheck" }

cppdialect "C++17"

filter {"toolset:clang", "language:C++"}
  buildoptions "-stdlib=libc++"
  linkoptions "-stdlib=libc++"
  linkoptions "-use-ld=lld"
filter {"toolset:clang", "configurations:debug"}
  buildoptions "-D_LIBCPP_DEBUG"
  --buildoptions "-fsanitize=thread"
  buildoptions "-fsanitize=address"
  linkoptions "-fsanitize=address"
  --buildoptions "-fsanitize=memory"
  --linkoptions "-fsanitize=memory"
filter {"toolset:clang", "configurations:release"}
  buildoptions "-flto=thin"
  linkoptions "-flto=thin"
  --flags { "LinkTimeOptimization" }

filter "configurations:debug"
  defines { "DEBUG" }
  symbols "On"

filter "configurations:release"
  --defines { "NDEBUG" }
  optimize "Full"
filter {}

-- build the libraries
include "libs/protobuf/lib.lua"
include "libs/botan/lib.lua"
include "libs/zstd/lib.lua"

project( "archivarius" )
  language "C++"
  kind "ConsoleApp"
  files {
     "src/*.h",
     "src/*.c++",
     -- protocol buffers generated files. refresh em by:
     -- protoc -I=. --cpp_out=. format.proto
     -- don't forget to use the appropriate version of protoc. have fun with the mess!
     "src/format.pb.h",
     "src/format.pb.cc",
  }

  pchheader "src/precomp.h"
  pchsource "src/precomp.c++"

  -- add library deps. includes, defines and links
  dofile "libs/protobuf/exports.lua"
  dofile "libs/botan/exports.lua"
  dofile "libs/zstd/exports.lua"
  includedirs {
    "libs"
  }

  filter "system:linux"
    links { "pthread" }
    links { "acl" }
  filter {}




