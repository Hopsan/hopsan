
project(HopsanCLI)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)

include(${CMAKE_CURRENT_LIST_DIR}/../helpers.cmake)

file(GLOB_RECURSE hopsancli_srcfiles *.cpp *.h *.hpp)

add_executable(hopsancli ${hopsancli_srcfiles})

target_include_directories(hopsancli PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../HopsanCore/dependencies/rapidxml>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/tclap/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Utilities>)

target_link_libraries(hopsancli hopsancore)
target_link_optional_libraries(hopsancli hopsanhdf5exporter)

set_target_properties(hopsancli PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS hopsancli
  RUNTIME DESTINATION bin
)
