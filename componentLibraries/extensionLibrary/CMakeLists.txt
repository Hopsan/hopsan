cmake_minimum_required(VERSION 3.0)
project(ExampleComponentLibrary)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)

set(targetname extensionlibrary)

file(GLOB_RECURSE headerfiles *.h *.hpp)

add_library(${targetname} SHARED extensionLibrary.cpp ${headerfiles})
target_include_directories(${targetname} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(${targetname} hopsancore)

install(TARGETS ${targetname}
  RUNTIME DESTINATION componentLibraries/extensionLibrary
  LIBRARY DESTINATION componentLibraries/extensionLibrary
)
