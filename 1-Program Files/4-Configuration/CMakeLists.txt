cmake_minimum_required(VERSION 4.1.2)

project(SecureJSONCommandServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1. PATH RESOLUTION
# This file is in '4-Configuration', so '..' is '1-Program Files'
get_filename_component(BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(SRC_DIR "${BASE_DIR}/1-Source Files/Application")
set(HDR_DIR "${BASE_DIR}/2-Header Files")
set(EXT_DIR "${BASE_DIR}/3-External Dependancies")

# 2. FILE GATHERING
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${HDR_DIR}/*.hpp")

# DEBUG STATUS CHECK
list(LENGTH SOURCES SRC_COUNT)
list(LENGTH HEADERS HDR_COUNT)
message(STATUS ">>> Sources found: ${SRC_COUNT} in ${SRC_DIR}")
message(STATUS ">>> Headers found: ${HDR_COUNT} in ${HDR_DIR}")

if(SRC_COUNT EQUAL 0)
    message(FATAL_ERROR "No source files found! Check path: ${SRC_DIR}")
endif()

# 3. DEFINE TARGET
# Listing ${HEADERS} here is what makes the CLion warning disappear
add_executable(application ${SOURCES} ${HEADERS})

# 4. INCLUDE PATHS
target_include_directories(application PRIVATE
        "${HDR_DIR}"
        "${EXT_DIR}"
        "${EXT_DIR}/Boost/include"
        "${EXT_DIR}/nlohmannjason"
)

# 5. OPTIONS
target_compile_definitions(application PRIVATE ASIO_STANDALONE)
target_compile_options(application PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(application PRIVATE -static-libgcc -static-libstdc++)
endif()