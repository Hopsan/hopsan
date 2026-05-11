project(HopsanHdf5Exporter)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/hdf5.cmake)

if (TARGET module_hdf5-shared)
  set(target_name hopsanhdf5exporter)

  # Find source code
  file(GLOB_RECURSE srcfiles ${CMAKE_CURRENT_LIST_DIR}/*.cpp
                             ${CMAKE_CURRENT_LIST_DIR}/*.h
                             ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

  # Create library target and add source code files
  add_library(${target_name} STATIC ${srcfiles})

  # Set include directories
  target_include_directories(${target_name} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
                             $<INSTALL_INTERFACE:include>)

  # Set link dependencies
  target_link_libraries(${target_name} hopsancore hdf5::hdf5-shared hdf5::hdf5_cpp-shared)
endif()
