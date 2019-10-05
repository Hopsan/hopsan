cmake_minimum_required(VERSION 3.0)
project(HopsanCoreTests)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)

set(test_name tst_generatortest)

add_executable(${test_name} ${test_name}.cpp)
target_compile_definitions(${test_name} PRIVATE
    TEST_DATA_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/\"
    DEFAULT_LIBRARY_ROOT=\"${CMAKE_CURRENT_BINARY_DIR}/../../componentLibraries/defaultLibrary/\"
    EXTERNAL_LIBRARIES_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/../../componentLibraries\"
    HOPSAN_INSTALL_ROOT=\"${CMAKE_CURRENT_BINARY_DIR}/../..\")
target_link_libraries(${test_name} hopsangenerator Qt5::Test)
add_test(NAME ${test_name} COMMAND ${test_name})

# Copy compiled hopsancore and source code to mimic installation
add_custom_command(
    TARGET tst_generatortest
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/../../bin"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:hopsancore>" "${CMAKE_CURRENT_BINARY_DIR}/../../bin"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/../../HopsanCore" "${CMAKE_CURRENT_BINARY_DIR}/../../HopsanCore"
)

# Copy default library source code to mimic installation
set(rel_default_lib ../../componentLibraries/defaultLibrary)
add_custom_command(
    TARGET tst_generatortest
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/${rel_default_lib}" "${CMAKE_CURRENT_BINARY_DIR}/${rel_default_lib}"
)

# Copy FMILibrary to mimic installation
set(rel_dependencies ../../Dependencies)
set(rel_fmilibrary ../../Dependencies/FMILibrary)
add_custom_command(
    TARGET tst_generatortest
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/${rel_dependencies}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/${rel_fmilibrary}" "${CMAKE_CURRENT_BINARY_DIR}/${rel_fmilibrary}"
)

# Copy FMUChecker to mimic installation
set(rel_tools ../../Dependencies/tools)
set(rel_fmuchecker ../../Dependencies/tools/FMUChecker)
add_custom_command(
    TARGET tst_generatortest
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/${rel_tools}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/${rel_fmuchecker}" "${CMAKE_CURRENT_BINARY_DIR}/${rel_fmuchecker}"
)
