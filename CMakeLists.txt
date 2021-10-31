cmake_minimum_required(VERSION 3.1)
project(ngram_porter)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-Wall -Wextra -g -DDEBUG -O0)
#add_compile_options(-Wall -Wextra -O2 -DASSERTF_DISABLE)

# TODO: Stringify CMAKE_CXX_FLAGS

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

add_library(${PROJECT_NAME} SHARED
        src/ngram_porter.cc
        src/utils.cc
        )
