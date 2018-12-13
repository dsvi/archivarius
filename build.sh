#!/bin/sh

#premake5 --cc=clang gmake2
premake5 gmake
make -j $(nproc) -C build all CXX=g++-8 CC=gcc-8
