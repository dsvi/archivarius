cmake_minimum_required(VERSION 3.22)

project(progress-bar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(progress-bar progress_bar.h progress_bar.c++)
target_include_directories(progress-bar PUBLIC .)
target_link_libraries(progress-bar PUBLIC coformat)

if (PROJECT_IS_TOP_LEVEL)
    include(FetchContent)
    FetchContent_Declare(
        cfmt
        GIT_REPOSITORY git@gitlab.com:44100Hz/coformat.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(cfmt)
    add_executable(progress_bar_example main.c++)
    target_link_libraries(progress_bar_example PRIVATE progress-bar)
endif()

include(GNUInstallDirs)
install(TARGETS progress-bar
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
