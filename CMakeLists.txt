cmake_minimum_required(VERSION 3.20)
project(CS50x C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set a default build type if none is specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Helpful for debugging (CLion usually does this in Debug, but explicit is fine)
# -fsanitize=address -fsanitize=leak
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak \
-Wall -Werror -Wno-unknown-attributes")

# Use add_compile_options with generator expressions to robustly add release flags
# This ensures they are applied even if the cache has old values.
add_compile_options($<$<CONFIG:Release>:-O3>)
add_compile_options($<$<CONFIG:Release>:-march=native>)
add_compile_options($<$<CONFIG:Release>:-DNDEBUG>)


# 1. Explicitly define your library/shared code
add_executable(string_buffer roblib/string_buffer.c roblib/string_buffer.h)

# Explicit target matching your manual clang command:
add_executable(test_hashmap2
        roblib/map/test_hashmap2.c
        roblib/map/hashmap.c
        roblib/collections.c
        roblib/map/string_counter.c
        roblib/memory/memory_pool.c
        roblib/munit/munit.c
)

target_compile_definitions(test_hashmap2 PRIVATE TEST_HASHMAP)
target_compile_options(test_hashmap2 PRIVATE -Wno-unused-function)

add_executable(string_counter
        roblib/map/test_string_counter.c
        roblib/map/string_counter.c
        roblib/collections.c
        roblib/munit/munit.c
        roblib/memory/memory_pool.c
        roblib/map/test_hashmap.c
        roblib/map/hashmap.c
)
target_compile_definitions(string_counter PRIVATE TEST_STRING_COUNTER)

add_executable(test_array_list
        roblib/list/test_array_list.c
        roblib/collections.c
        roblib/list/array_list.c
        roblib/memory/memory_pool.c
        roblib/munit/munit.c
)

target_compile_definitions(test_array_list PRIVATE TEST_ARRAY_LIST)


add_executable(all_tests
        roblib/all_tests.c
        roblib/collections.c
        roblib/munit/munit.c
        roblib/memory/memory_pool.c
        roblib/map/test_hashmap.c
        roblib/map/hashmap.c
        roblib/map/string_counter.c
        roblib/map/test_string_counter.c
        roblib/list/test_array_list.c
        roblib/list/array_list.c
)

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


