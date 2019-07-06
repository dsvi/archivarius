 cmake_minimum_required(VERSION 3.14)
 
# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE)
endif ()



project(botan  
    VERSION 2.9 
    DESCRIPTION "Botan crypto library")

add_library(botan STATIC botan_all.h botan_all.cpp botan_all_internal.h)
target_include_directories(botan PUBLIC .)
target_compile_features(botan PRIVATE cxx_std_17)
target_compile_options(botan PUBLIC -mavx2)
