cmake_minimum_required(VERSION 3.1)
project(hopsandcp)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

set(CMAKE_AUTOMOC ON)

# Find source code
file(GLOB_RECURSE srcfiles src/*.cpp)
file(GLOB_RECURSE headerfiles include/*.h include/*.hpp)

# Create library target and add source code files
add_library(hopsandcp STATIC ${srcfiles} ${headerfiles})

# Set include directories
target_include_directories(hopsandcp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../HopsanCore/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/dcplib-code/include/bluetooth>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/dcplib-code/include/core>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/dcplib-code/include/ethernet>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/dcplib-code/include/master>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/dcplib-code/include/slave>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/dcplib-code/include/xml>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/dcplib-code/include/zip>
  $<INSTALL_INTERFACE:include>)
