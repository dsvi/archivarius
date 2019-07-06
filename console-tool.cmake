include(lib.cmake)

set(archivarius-tool-srcs 
./archivarius/tool/cmd_line_parser.c++
./archivarius/tool/cmd_line_parser.h
./archivarius/tool/config.c++
./archivarius/tool/config.h
./archivarius/tool/main.c++
./archivarius/tool/property_tree.c++
./archivarius/tool/property_tree.h
./archivarius/tool/test.c++
./archivarius/tool/test.h
)


add_executable(archivarius ${archivarius-tool-srcs})
target_include_directories(archivarius PRIVATE src)
set_target_properties(archivarius PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(archivarius archivarius-lib)


