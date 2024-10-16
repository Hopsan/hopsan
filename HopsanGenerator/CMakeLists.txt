
project(HopsanGenerator)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

find_package(Qt5 COMPONENTS Core Xml REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(target_name hopsangenerator)

include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/fmilibrary.cmake)

file(GLOB_RECURSE srcfiles src/*.cpp include/*.h include/*.hpp templates.qrc)

add_library(${target_name} SHARED ${srcfiles})

# Set compiler definitions
if(WIN32)
  target_compile_definitions(${target_name} PRIVATE HOPSANGENERATOR_DLLEXPORT)
  target_compile_definitions(${target_name} INTERFACE HOPSANGENERATOR_DLLIMPORT)
endif()
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
  # Silence qDebug() for all non Debug configurations
  target_compile_definitions(${target_name} PRIVATE QT_NO_DEBUG_OUTPUT)
endif()

target_include_directories(${target_name} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(${target_name} hopsancore fmilibrary symhop Qt5::Core Qt5::Xml)

set_target_properties(${target_name} PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS ${target_name}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
