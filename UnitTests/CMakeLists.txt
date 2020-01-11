find_package(Qt5 COMPONENTS Test Core REQUIRED)
set(CMAKE_AUTOMOC ON)

include(${CMAKE_CURRENT_LIST_DIR}/../helpers.cmake)

add_cmake_subdirectories(${CMAKE_CURRENT_LIST_DIR})
