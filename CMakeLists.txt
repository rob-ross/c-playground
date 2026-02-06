cmake_minimum_required(VERSION 3.20)
project(CS50x C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 1. Explicitly define your library/shared code
add_executable(string_buffer roblib/string_buffer.c roblib/string_buffer.h)

# 2. AUTOMATION: Find all other .c files and make them executables
# This is great for "learning" repos where every file has a main()
file(GLOB_RECURSE ALL_SOURCES "*.c")
foreach(SOURCE_FILE ${ALL_SOURCES})
    get_filename_component(TARGET_NAME ${SOURCE_FILE} NAME_WE)
    # Check if target already exists to avoid duplicates (like string_buffer above)
    if(NOT TARGET ${TARGET_NAME})
        add_executable(${TARGET_NAME} ${SOURCE_FILE})
    endif()
endforeach()
