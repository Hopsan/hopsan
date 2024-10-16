project(HopsanRemote)

include(${CMAKE_CURRENT_LIST_DIR}/../helpers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/zeromq.cmake)

if (TARGET libzmq)
  add_cmake_subdirectories(${CMAKE_CURRENT_LIST_DIR})
endif()
