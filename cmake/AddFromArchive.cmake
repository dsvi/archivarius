cmake_minimum_required(VERSION 3.18)
include(FetchContent)

function(add_from_archive_url archive_url)
    if(NOT ${ARGC} EQUAL 1)
        set(subdir ${ARGV1})
    endif()
    get_filename_component(fc_name ${archive_url} NAME)
    message("Adding ${fc_name}")
    FetchContent_Declare(
        ${fc_name}
        URL ${archive_url}
        SOURCE_SUBDIR ${subdir}
    )
    FetchContent_MakeAvailable(${fc_name})
endfunction()

# archive_rel_path path to archive relative to current CMakeLists.txt
function(add_from_archive archive_rel_path)
    if(NOT ${ARGC} EQUAL 1)
        set(subdir ${ARGV1})
    endif()
    add_from_archive_url(${CMAKE_CURRENT_LIST_DIR}/${archive_rel_path} ${subdir})
endfunction()
