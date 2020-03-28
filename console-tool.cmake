include(lib.cmake)

configure_file(${PROJECT_SOURCE_DIR}/version.h ${PROJECT_SOURCE_DIR}/archivarius/cli/)

set(archivarius-tool-srcs 
./archivarius/cli/cmd_line_parser.c++
./archivarius/cli/cmd_line_parser.h
./archivarius/cli/config.c++
./archivarius/cli/config.h
./archivarius/cli/main.c++
./archivarius/cli/property_tree.c++
./archivarius/cli/property_tree.h
./archivarius/cli/test.c++
./archivarius/cli/test.h
./archivarius/cli/version.h
)


add_executable(archivarius ${archivarius-tool-srcs})
target_include_directories(archivarius PRIVATE src)
set_target_properties(archivarius PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(archivarius archivarius-lib)


