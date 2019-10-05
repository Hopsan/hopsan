cmake_minimum_required(VERSION 3.0)
project(DefaultLibraryXMLTest)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)
find_package(Qt5 COMPONENTS Xml REQUIRED)

set(test_name tst_defaultlibraryxmltest)

add_executable(${test_name} ${test_name}.cpp)
target_compile_definitions(${test_name} PRIVATE
  DEFAULT_LIBRARY_ROOT=\"${CMAKE_CURRENT_BINARY_DIR}/../../componentLibraries/defaultLibrary/\"
  TEST_DATA_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/\")
target_link_libraries(${test_name} hopsancore Qt5::Test Qt5::Xml)
add_test(${test_name} ${test_name})
