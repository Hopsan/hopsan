cmake_minimum_required(VERSION 3.0)
project(HopsanCLI)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)

file(GLOB_RECURSE hopsancli_srcfiles *.cpp)
file(GLOB_RECURSE hopsancli_incfiles *.h *.hpp)

add_executable(HopsanCLI ${hopsancli_srcfiles})

target_include_directories(HopsanCLI PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../HopsanCore/dependencies/rapidxml>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/tclap/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Utilities>)

target_link_libraries(HopsanCLI HopsanCore)

set_target_properties(HopsanCLI PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS HopsanCLI
  RUNTIME DESTINATION bin
)
