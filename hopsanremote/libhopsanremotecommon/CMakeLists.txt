cmake_minimum_required(VERSION 3.0)
project(Ops)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

set(target_name libhopsanremotecommon)

include(${CMAKE_CURRENT_LIST_DIR}/../../dependencies/zeromq.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../dependencies/msgpack.cmake)

file(GLOB_RECURSE srcfiles src/*.cpp include/*.h include/*.hpp)

add_library(${target_name} STATIC ${srcfiles})
set_target_properties(${target_name} PROPERTIES OUTPUT_NAME hopsanremotecommon)

target_include_directories(${target_name} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(${target_name} PUBLIC libzmq msgpackc-cxx)

install(TARGETS ${target_name}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include )
