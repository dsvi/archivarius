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

target_link_libraries(archivarius archivarius-lib)


