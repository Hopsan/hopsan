project(HopsanHdf5Exporter)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../dependencies/hdf5.cmake)

if (TARGET module_hdf5-shared)
  # Find source code
  file(GLOB_RECURSE srcfiles ${CMAKE_CURRENT_LIST_DIR}/*.cpp
                             ${CMAKE_CURRENT_LIST_DIR}/*.h
                             ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

  # Create library target and add source code files
  add_library(hopsanhdf5exporter STATIC ${srcfiles})

  if(MSVC)
    # Enable multi-core build with MSVC
    target_compile_options(hopsanhdf5exporter PRIVATE "/MP")
  endif()

  # Set include directories
  target_include_directories(hopsanhdf5exporter PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
                             $<INSTALL_INTERFACE:include>)

  # Set link dependencies
  target_link_libraries(hopsanhdf5exporter hopsancore hdf5::hdf5-shared hdf5::hdf5_cpp-shared)
endif()
