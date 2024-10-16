project(HopsanCoreTests)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)

set(test_name tst_componentutilitiestesttest)

add_executable(${test_name} ${test_name}.cpp)
target_link_libraries(${test_name} hopsancore Qt5::Test)
add_test(${test_name} ${test_name})

if (WIN32)
    copy_file_after_build(${test_name} $<TARGET_FILE:hopsancore> $<TARGET_FILE_DIR:${test_name}>)
endif()
