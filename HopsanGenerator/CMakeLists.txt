
project(HopsanGenerator)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

find_package(Qt5 COMPONENTS Core Xml REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE srcfiles src/*.cpp include/*.h include/*.hpp templates.qrc)

add_library(hopsangenerator SHARED ${srcfiles})

if(MSVC)
  # Enable multi-core build with MSVC
  target_compile_options(hopsangenerator PRIVATE "/MP")
endif()

# Set compiler definitions
if(WIN32)
  target_compile_definitions(hopsangenerator PRIVATE HOPSANGENERATOR_DLLEXPORT)
  target_compile_definitions(hopsangenerator INTERFACE HOPSANGENERATOR_DLLIMPORT)
endif()
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
  # Silence qDebug() for all non Debug configurations
  target_compile_definitions(hopsangenerator PRIVATE QT_NO_DEBUG_OUTPUT)
endif()

target_include_directories(hopsangenerator PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(hopsangenerator hopsancore symhop Qt5::Core Qt5::Xml)

set_target_properties(hopsangenerator PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS hopsangenerator
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
