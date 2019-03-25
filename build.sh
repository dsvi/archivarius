#!/bin/sh

export CC="gcc-8"
bazel build --cxxopt='-std=c++17' --cxxopt='-mavx2' archivarius --verbose_failures
