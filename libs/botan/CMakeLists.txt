cmake_minimum_required(VERSION 3.14)
 

project(botan  
    VERSION 2.13.0 
    DESCRIPTION "Botan crypto library")

add_library(botan STATIC botan_all.h botan_all.cpp botan_all_internal.h)
target_include_directories(botan PUBLIC .)
set_target_properties(botan PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
# avx2 optimizations dont seem to benefit much to the whole tool performance
#target_compile_options(botan PUBLIC -march=native)
