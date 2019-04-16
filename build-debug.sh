#!/bin/sh

#bazel clean --expunge
export CC="gcc-8"
bazel build --cxxopt='-std=c++17' --cxxopt='-mavx2' archivarius --verbose_failures --compilation_mode dbg
#export CC="clang-7"
#bazel build --cxxopt=-std=c++17 --cxxopt='-mavx2' archivarius --verbose_failures --compilation_mode dbg --linkopt=-stdlib=libc++

