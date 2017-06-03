cmake_minimum_required(VERSION 3.0)
project(defaultComponentLibrary)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)


#file(GLOB_RECURSE srcfiles src/*.cpp)
#file(GLOB_RECURSE incfiles include/*.h include/*.hpp)

add_library(defaultComponentLibrary SHARED defaultComponentLibrary.cpp defaultComponentLibraryInternal.cpp)
target_include_directories(defaultComponentLibrary PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(defaultComponentLibrary HopsanCore)

install(TARGETS defaultComponentLibrary
  RUNTIME DESTINATION componentLibraries/defaultLibrary
  LIBRARY DESTINATION componentLibraries/defaultLibrary
)
