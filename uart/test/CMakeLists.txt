cmake_minimum_required(VERSION 3.14)
project(UartDeviceDriverTest VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Place targets in bin/ or lib/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Build a test application for counter_vn29a80.ko
add_executable(test_counter_vn29a80
    "vn29a80_driver_tester.cpp"
    "vn29a80_driver_testcases.cpp"
)
target_compile_options(test_counter_vn29a80 PRIVATE -Wall -O2)
target_link_libraries(test_counter_vn29a80 m pthread gtest_main gtest gmock)
