cmake_minimum_required(VERSION 3.0)
project(Ops)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)

file(GLOB_RECURSE ops_srcfiles src/*.cpp )

add_library(Ops SHARED ${ops_srcfiles})
target_include_directories(Ops PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
  $<INSTALL_INTERFACE:include>)

install(TARGETS Ops
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include )