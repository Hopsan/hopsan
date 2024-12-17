project(HopsanCoreTests)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)

set(test_name tst_lookuptabletest)

add_executable(${test_name} ${test_name}.cpp)
target_compile_definitions(${test_name} PRIVATE TEST_DATA_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/\")
target_link_libraries(${test_name} hopsancore Qt5::Test Qt5::Core)
add_test(NAME ${test_name} COMMAND ${test_name})
