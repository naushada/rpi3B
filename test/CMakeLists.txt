cmake_minimum_required(VERSION 3.16.3)
project(rpi3B)
find_package(GTest REQUIRED)


include_directories(../inc)
include_directories(test)

#Adding Complier flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -g -Wall -Wextra")

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "*.cpp"  
                    "../src/gpio/gpio.cpp"
                    "../src/clock/clock.cpp"
                    "../src/interrupt/interrupt.cpp"
                    )

enable_testing()

add_executable(rpi3B_test ${SOURCES})
target_link_libraries(rpi3B_test gtest_main)

target_link_libraries(rpi3B_test gtest)
target_link_libraries(rpi3B_test pthread)

add_test(rpi3B_gtests rpi3B_test)
gtest_discover_tests(rpi3B_test)
