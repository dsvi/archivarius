cmake_minimum_required(VERSION 3.14)
include(FetchContent)

if (NOT BOTAN_ARCHIVE)
    message(FATAL_ERROR "gotta set BOTAN_ARCHIVE")
endif()

if (NOT BOTAN_MODULES_LIST)
    message(FATAL_ERROR "gotta set BOTAN_MODULES_LIST")
endif()

list(JOIN BOTAN_MODULES_LIST "," ENABLE_MODULES_LIST)

FetchContent_Declare(
    botan_upstream
    URL ${CMAKE_CURRENT_LIST_DIR}/${BOTAN_ARCHIVE}
)
FetchContent_MakeAvailable(botan_upstream)

# Determine botan compiler ID (--cc parameter of configure.py)
set(BOTAN_COMPILER_ID ${CMAKE_CXX_COMPILER_ID})
string(TOLOWER ${BOTAN_COMPILER_ID} BOTAN_COMPILER_ID)
if (BOTAN_COMPILER_ID STREQUAL "gnu")
    set(BOTAN_COMPILER_ID "gcc")
endif()

execute_process(COMMAND python
    ${botan_upstream_SOURCE_DIR}/configure.py
    --quiet
    --cc-bin=${CMAKE_CXX_COMPILER}
    --cc=${BOTAN_COMPILER_ID}
    --disable-shared
    --amalgamation
    --minimized-build
    --enable-modules=${ENABLE_MODULES_LIST}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RESULT_VARIABLE CONFIG_RESULT)
    if(NOT CONFIG_RESULT EQUAL "0")
        message(FATAL_ERROR "./configure.py failed with ${CONFIG_RESULT}")
    endif()

add_library(botan STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/botan_all.h
    ${CMAKE_CURRENT_BINARY_DIR}/botan_all.cpp
)
target_include_directories(botan PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

