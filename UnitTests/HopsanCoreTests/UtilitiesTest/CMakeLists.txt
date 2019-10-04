cmake_minimum_required(VERSION 3.0)
project(HopsanCoreTests)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)

set(test_name tst_utilitiestesttest)

add_executable(${test_name} ${test_name}.cpp)
target_link_libraries(${test_name} hopsancore Qt5::Test)
add_test(${test_name} ${test_name})
