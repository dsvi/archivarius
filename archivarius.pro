 
TEMPLATE = app
QMAKE_EXT_CPP += c++

#target.path = /usr/bin
#INSTALLS += target
#desktop.path = /usr/share/applications
#desktop.files = NotesTree.desktop
#INSTALLS += desktop
#icons.path = /usr/share/icons/hicolor/scalable/apps
#icons.files = NotesTree.svg
#INSTALLS += icons

CONFIG += precompile_header
CONFIG += c++1z
#INCLUDEPATH += /usr/include/c++/8/
INCLUDEPATH += /usr/lib/llvm-7/include/c++/v1/

CONFIG(debug, debug|release) {
  DEFINES += DEBUG
}

CONFIG(release, debug|release) {
  DEFINES += NDEBUG
}

PRECOMPILED_HEADER = src/precomp.h
INCLUDEPATH += src
INCLUDEPATH += libs/fmt/include
INCLUDEPATH += libs/botan
INCLUDEPATH += libs/protobuf/src
INCLUDEPATH += libs/range-v3

SOURCES += \
src/cmd_line_parser.c++ \
src/cmd_line_parser.h \ 
src/archive.c++ \
src/archive.h \ 
src/buffer.c++ \
src/buffer.h \
src/catalogue.c++ \
src/catalogue.h  \
src/config.c++  \
src/config.h  \
src/exception.c++ \
src/exception.h \
src/file_content_creator.c++ \
src/file_content_creator.h \
src/file_content_ref.h \
src/filesystem_state.c++ \
src/filesystem_state.h \
src/piping.c++ \
src/piping.h \
src/piping_aes.c++ \
src/piping_aes.h \
src/piping_xxhash.c++ \
src/piping_xxhash.h \ 
src/piping_zstd.c++ \
src/piping_zstd.h \
src/platform.c++ \
src/platform.h \
src/restore.c++ \
src/restore.h \
src/stream.c++ \
src/stream.h \
src/zstd_filter.c++ \
src/zstd_filter.h \
src/format.proto \
src/main.c++ \
src/precomp.h \ 
src/precomp.c++ \
src/globals.h \
src/globals.c++ \
src/format.pb.h \
src/format.pb.cc \
src/property_tree.c++ \
src/property_tree.h



