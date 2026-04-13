cmake_minimum_required(VERSION 3.14)
project(esp32_thing_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GoogleTest via FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

# Include paths: mocks/ must come first so sketch's angle-bracket includes resolve to mocks
set(MOCK_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Shared sketch library (compiled once, linked into each test executable)
# mock_support.cpp is a separate TU so its definitions are available to all tests
add_library(sketch_lib OBJECT
    sketch_wrapper.cpp
    mocks/mock_support.cpp
)
target_include_directories(sketch_lib PUBLIC ${MOCK_INCLUDES})
target_compile_definitions(sketch_lib PUBLIC UNIT_TEST=1)

# Helper macro: create a test executable linked against sketch_lib
function(add_sketch_test target_name source_file)
    add_executable(${target_name} ${source_file})
    target_include_directories(${target_name} PRIVATE ${MOCK_INCLUDES})
    target_compile_definitions(${target_name} PRIVATE UNIT_TEST=1)
    target_link_libraries(${target_name}
        PRIVATE
        $<TARGET_OBJECTS:sketch_lib>
        GTest::gtest_main
    )
    gtest_discover_tests(${target_name}
        XML_OUTPUT_DIR ${CMAKE_BINARY_DIR}/test-results
    )
endfunction()

add_sketch_test(test_config       test_config.cpp)
add_sketch_test(test_web_handlers test_web_handlers.cpp)
add_sketch_test(test_gpio         test_gpio.cpp)
