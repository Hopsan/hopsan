project(hopsandcp)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

set(CMAKE_AUTOMOC ON)

# include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/xerces.cmake)
# include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/libzip.cmake)
# include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/zlib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/libzip.cmake)
#include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/zlib.cmake)

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
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/libzip/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/asio-code/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/xerces/include>
  $<INSTALL_INTERFACE:include>)

if(WIN32)
    target_link_libraries(hopsandcp hopsancore libzip -lws2_32)
else()
    target_link_libraries(hopsandcp hopsancore libzip)
endif()

target_compile_definitions(hopsandcp PUBLIC LOGGING=ON)

