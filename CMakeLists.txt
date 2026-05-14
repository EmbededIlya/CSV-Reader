cmake_minimum_required(VERSION 3.14)

project(
    CSV_READER
    VERSION 1.0.0
    DESCRIPTION "CSV formula evaluator"
    LANGUAGES C
)

# =========================
# C standard
# =========================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# =========================
# Build type
# =========================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# =========================
# Source files
# =========================

set(SOURCES
    src/main.c
    src/csv.c
    src/error.c
    src/evaluator.c
    src/parser.c
    src/table.c
)

# =========================
# Header files
# =========================

set(HEADERS
    inc/csv.h
    inc/error.h
    inc/evaluator.h
    inc/parser.h
    inc/table.h
)

# =========================
# File existence checks
# =========================

foreach(FILE ${SOURCES})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
        message(FATAL_ERROR "Source file not found: ${FILE}")
    endif()
endforeach()

# =========================
# Executable
# =========================

add_executable(csvreader
    ${SOURCES}
    ${HEADERS}
)

# =========================
# Include directories
# =========================

target_include_directories(csvreader PRIVATE inc)

# =========================
# Compiler warnings
# =========================

target_compile_options(csvreader PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

# =========================
# Debug helper
# =========================

function(enable_debug target_name)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")

        target_compile_options(${target_name} PRIVATE
            -g
            -O0
            -fno-omit-frame-pointer
        )

    endif()

endfunction()


# =========================
# Output directories
# =========================

set_target_properties(csvreader PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# =========================
# Project info
# =========================

message(STATUS "Project name: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")

# =========================
# Testing
# =========================
add_executable(table_test 
    tests/table_test.c 
    src/table.c 
    src/error.c
    src/evaluator.c
)


enable_debug(table_test)


target_include_directories(table_test PRIVATE inc)
enable_testing()