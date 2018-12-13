#!/bin/sh

#premake5 --cc=clang gmake2
#premake5 gmake
#make -j $(nproc) -C build all config=debug CXX=g++-8 CC=gcc-8
export CC="gcc-8"
bazel build archivarius
