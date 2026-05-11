project(HopsanRemote)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)

include(${CMAKE_CURRENT_LIST_DIR}/../helpers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/zeromq.cmake)

if (TARGET libzmq)
  add_cmake_subdirectories(${CMAKE_CURRENT_LIST_DIR})
endif()
