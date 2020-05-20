cmake_minimum_required(VERSION 3.1)
project(hopsanc)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

# Find source code
file(GLOB_RECURSE srcfiles src/*.cpp)
file(GLOB_RECURSE headerfiles include/*.h include/*.hpp)

# Create library target and add source code files
add_library(hopsanc SHARED ${srcfiles} ${headerfiles})

# Set include directories
target_include_directories(hopsanc PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../HopsanCore/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

# Set system link dependencies
target_link_libraries(hopsancore)
