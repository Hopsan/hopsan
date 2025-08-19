project(ExampleComponentLibrary)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

set(targetname extensionlibrary)

include(${CMAKE_CURRENT_LIST_DIR}/../../dependencies/fmilibrary.cmake)

file(GLOB_RECURSE component_files *.h *.hpp *cci)

add_library(${targetname} SHARED extensionLibrary.cpp ${component_files})
target_include_directories(${targetname} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(${targetname} hopsancore fmilibrary)

install(TARGETS ${targetname}
  RUNTIME DESTINATION componentLibraries/extensionLibrary
  LIBRARY DESTINATION componentLibraries/extensionLibrary
)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ DESTINATION componentLibraries/extensionLibrary
  FILES_MATCHING
  PATTERN "*.cpp"
  PATTERN "*.h"
  PATTERN "*.hpp"
  PATTERN "*.cci"
  PATTERN "*.xml"
  PATTERN "*.nb"
  PATTERN "*.py"
  PATTERN "*.bat"
  PATTERN "*.sh"
  PATTERN "*.csv"
  PATTERN "*.txt"
  PATTERN "*.md"
  PATTERN "*.hmf"
  PATTERN "*.svg"
  PATTERN "LICENSE"
  PATTERN "CMakeLists.txt" EXCLUDE)
