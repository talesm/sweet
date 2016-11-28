cmake_minimum_required(VERSION 3.1)
project(Sweet)

add_executable(sweet
    src/main
)

target_compile_options(sweet
    PRIVATE -Wall -Werror -pedantic
)
