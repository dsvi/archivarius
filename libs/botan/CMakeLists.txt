cmake_minimum_required(VERSION 3.14)
 

project(botan  
    VERSION 2.9 
    DESCRIPTION "Botan crypto library")

add_library(botan STATIC botan_all.h botan_all.cpp botan_all_internal.h)
target_include_directories(botan PUBLIC .)
target_compile_features(botan PRIVATE cxx_std_17)
# avx2 optimizations dont seem to benefit much to the whole tool performance
#target_compile_options(botan PUBLIC -march=native)
