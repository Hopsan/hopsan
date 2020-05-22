cmake_minimum_required(VERSION 3.0)
project(HopsanCLI)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)

file(GLOB_RECURSE hopsancli_srcfiles *.cpp)
file(GLOB_RECURSE hopsancli_incfiles *.h *.hpp)

add_executable(hopsancli ${hopsancli_srcfiles})

target_include_directories(hopsancli PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../HopsanCore/dependencies/rapidxml>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/tclap/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Utilities>)

target_link_libraries(hopsancli hopsancore)

set_target_properties(hopsancli PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS hopsancli
  RUNTIME DESTINATION bin
)
