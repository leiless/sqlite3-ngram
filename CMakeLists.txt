cmake_minimum_required(VERSION 3.1)
project(ngram)

set(CMAKE_CXX_STANDARD 11)

# TODO: Stringify CMAKE_CXX_FLAGS
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -g -DDEBUG -O0)
else ()
    add_compile_options(-Wall -Wextra -O2)
endif ()

execute_process(
        COMMAND git describe --dirty --always --abbrev=7
        OUTPUT_VARIABLE BUILD_HEAD_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(BUILD_HEAD_COMMIT="${BUILD_HEAD_COMMIT}")

execute_process(
        COMMAND date "+%Y/%m/%d %H:%M:%S %z"
        OUTPUT_VARIABLE BUILD_TIMESTAMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(BUILD_TIMESTAMP="${BUILD_TIMESTAMP}")

execute_process(
        COMMAND whoami
        OUTPUT_VARIABLE BUILD_USER
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(BUILD_USER="${BUILD_USER}")

# see: https://github.com/google/glog#incorporating-glog-into-a-cmake-project
find_package(glog 0.6.0 REQUIRED)
#find_package(Protobuf 2.6.1 REQUIRED)
find_library(LIBPROTOBUF NAMES libprotobuf.a protobuf)

add_library(
        ${PROJECT_NAME} SHARED
        src/ngram.cpp
        src/utils.cpp
        src/token_vector.cpp
        src/highlight.cpp
        src/proto/highlight_result.pb.cc
)

target_link_libraries(${PROJECT_NAME} glog::glog ${LIBPROTOBUF})
