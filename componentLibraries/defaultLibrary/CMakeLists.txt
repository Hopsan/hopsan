project(DefaultComponentLibrary)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)

include(${CMAKE_CURRENT_LIST_DIR}/../../dependencies/fmi4c.cmake)

file(GLOB_RECURSE component_files *.cpp *.cci *.h *.hpp)

add_library(defaultcomponentlibrary SHARED ${component_files})
target_include_directories(defaultcomponentlibrary PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(defaultcomponentlibrary hopsancore)
target_link_optional_libraries(defaultcomponentlibrary fmi4c::fmi4c)
if(WIN32)
  set_target_properties(defaultcomponentlibrary PROPERTIES
                                                PREFIX ""
                                                IMPORT_PREFIX "")
endif()

install(TARGETS defaultcomponentlibrary
  RUNTIME DESTINATION componentLibraries/defaultLibrary
  LIBRARY DESTINATION componentLibraries/defaultLibrary
)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ DESTINATION componentLibraries/defaultLibrary
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
