cmake_minimum_required(VERSION 3.1)
project(Sweet)

find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(sweet
    src/main
)

target_include_directories(sweet
    PRIVATE ${Boost_INCLUDE_DIRS}
)

target_link_libraries(sweet
    PRIVATE ${Boost_LIBRARIES}
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
