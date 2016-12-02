cmake_minimum_required(VERSION 3.1)
project(Sweet)

add_executable(sweet
    src/main
)

target_compile_options(sweet
    PRIVATE -Wall -Werror -pedantic
)

add_executable(sweet_tests
    test/catch
    test/FileTargetTest
    test/MemoryTargetTest
)

target_compile_definitions(sweet_tests
    PRIVATE TEST_TEMP_PREFIX="${CMAKE_CURRENT_BINARY_DIR}"
)
