cmake_minimum_required(VERSION 3.20)
project(ATPO_v2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
file(GLOB_RECURSE HEADER_FILES  CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/header/*.h)

add_executable(ATPO_v2 ${SOURCES} ${HEADER_FILES})
    
target_include_directories(ATPO_v2 PRIVATE ${CMAKE_SOURCE_DIR}/header)

if (MSVC)
    target_compile_options(ATPO_v2 PRIVATE /W4 /permissive-)
else()
    target_compile_options(ATPO_v2 PRIVATE -Wall -Wextra -Wpedantic)
endif()

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCES})
