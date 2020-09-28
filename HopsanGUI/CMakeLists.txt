cmake_minimum_required(VERSION 3.0)
project(HopsanGUI)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets Xml Svg Network Test PrintSupport REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/zeromq.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/msgpack.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/qwt.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/discount.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/katex.cmake)

set(target_name hopsangui)

file(GLOB_RECURSE srcfiles *.cpp *.h *.hpp *.qrc)
add_executable(${target_name} ${srcfiles})

target_include_directories(${target_name} PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../Utilities>)

target_link_libraries(${target_name} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Xml Qt5::Svg Qt5::Network Qt5::Test Qt5::PrintSupport
  qwt libzmq discount hopsancore hopsanhdf5exporter hopsangeneratorgui libhopsanremoteclient symhop ops)

set_target_properties(${target_name} PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS ${target_name}
  RUNTIME DESTINATION bin
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/../hopsan-default-configuration.xml
        DESTINATION .)
