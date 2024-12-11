project(HopsanCoreTests)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)

set(test_name tst_simulationtest)

add_executable(${test_name} ${test_name}.cpp)
target_compile_definitions(${test_name} PRIVATE
  DEFAULT_LIBRARY_ROOT=\"${CMAKE_CURRENT_BINARY_DIR}/../../../componentLibraries/defaultLibrary/\"
  TEST_DATA_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/\")
target_link_libraries(${test_name} hopsancore Qt5::Test)
add_test(${test_name} ${test_name})

if (WIN32)
    copy_file_after_build(${test_name} $<TARGET_FILE:hopsancore> $<TARGET_FILE_DIR:${test_name}>)
endif()
