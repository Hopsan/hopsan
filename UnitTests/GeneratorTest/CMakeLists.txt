project(HopsanCoreTests)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)

set(test_name tst_generatortest)

add_executable(${test_name} ${test_name}.cpp)
target_compile_definitions(${test_name} PRIVATE
    TEST_DATA_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/\"
    DEFAULT_LIBRARY_ROOT=\"${CMAKE_CURRENT_BINARY_DIR}/../../componentLibraries/defaultLibrary/\"
    EXTERNAL_LIBRARIES_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/../../componentLibraries\"
    HOPSAN_INSTALL_ROOT=\"${CMAKE_CURRENT_BINARY_DIR}/../../\")
target_link_libraries(${test_name} hopsangenerator Qt5::Test)
if (MSVC)
    message(WARNING "Skipping ${test_name} for MSVC")
else()
    add_test(NAME ${test_name} COMMAND ${test_name})
endif()

# Copy runtime requirement dll's to run directory
if (WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/../../dependencies/fmilibrary.cmake)
    copy_file_after_build(${test_name} $<TARGET_FILE:hopsancore> $<TARGET_FILE_DIR:${test_name}>)
    copy_file_after_build(${test_name} $<TARGET_FILE:hopsangenerator> $<TARGET_FILE_DIR:${test_name}>)
    copy_file_after_build(${test_name} $<TARGET_FILE:fmilibrary> $<TARGET_FILE_DIR:${test_name}>)
endif()

# Copy compiled hopsancore and source code to mimic installation
copy_file_after_build(${test_name} $<TARGET_FILE:hopsancore> ${CMAKE_CURRENT_BINARY_DIR}/../../bin)
copy_dir_after_build(${test_name} ${CMAKE_CURRENT_LIST_DIR}/../../HopsanCore ${CMAKE_CURRENT_BINARY_DIR}/../../)

# Copy default library source code to mimic installation
copy_dir_after_build(${test_name} ${CMAKE_CURRENT_LIST_DIR}/../../componentLibraries/defaultLibrary ${CMAKE_CURRENT_BINARY_DIR}/../../componentLibraries)
if (MSVC)
  # Copy default library dll to mimic installation
  copy_file_after_build(${test_name} $<TARGET_FILE:defaultcomponentlibrary> ${CMAKE_CURRENT_BINARY_DIR}/../../componentLibraries/defaultLibrary)
endif()

# Copy FMILibrary to mimic installation
copy_dir_after_build(${test_name} ${CMAKE_CURRENT_LIST_DIR}/../../dependencies/fmilibrary ${CMAKE_CURRENT_BINARY_DIR}/../../dependencies)

# Copy Fmi4c library to mimic installation (optional, but fmu export will not work without it)
set(fmi4c_dir ${CMAKE_CURRENT_LIST_DIR}/../../dependencies/fmi4c)
if (EXISTS ${fmi4c_dir})
  copy_dir_after_build(${test_name} ${fmi4c_dir} ${CMAKE_CURRENT_BINARY_DIR}/../../dependencies)
endif()

# Copy FMUChecker and 7z to mimic installation
copy_dir_after_build(${test_name} ${CMAKE_CURRENT_LIST_DIR}/../../dependencies/tools/FMUChecker ${CMAKE_CURRENT_BINARY_DIR}/../../dependencies/tools)
copy_dir_after_build(${test_name} ${CMAKE_CURRENT_LIST_DIR}/../../dependencies/tools/7z ${CMAKE_CURRENT_BINARY_DIR}/../../dependencies/tools)
