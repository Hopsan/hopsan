cmake_minimum_required(VERSION 3.0)
project(HopsanGenerator)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

find_package(Qt5 COMPONENTS Core Xml REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(target_name hopsangenerator)

include(${CMAKE_CURRENT_LIST_DIR}/../Dependencies/fmilibrary.cmake)

file(GLOB_RECURSE srcfiles src/*.cpp templates.qrc)
file(GLOB_RECURSE headerfiles include/*.h include/*.hpp)

add_library(${target_name} SHARED ${srcfiles} ${headerfiles})
target_include_directories(${target_name} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(${target_name} hopsancore fmilibrary symhop Qt5::Core Qt5::Xml)

install(TARGETS ${target_name}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
