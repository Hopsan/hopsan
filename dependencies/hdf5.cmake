set(local_hdf5_dir ${CMAKE_CURRENT_LIST_DIR}/hdf5)
find_package(Hdf5 CONFIG COMPONENTS C CXX PATHS ${local_hdf5_dir}/cmake ${local_hdf5_dir}/share/cmake NO_DEFAULT_PATH)
if (NOT Hdf5_FOUND)
  message(STATUS "Looking for CMake Hdf5 in system")
  find_package(Hdf5 CONFIG COMPONENTS C CXX)
  if (NOT Hdf5_FOUND)
    # Fall back to FindHDF5 Module
    message(STATUS "Looking for HDF5 Module in system")
    find_package(HDF5 COMPONENTS C CXX)
  endif()
endif()

if (Hdf5_FOUND)
  message(STATUS "Building with HDF5 support")
  #target_compile_definitions(hdf5::hdf5-shared INTERFACE USEHDF5)
  set_target_properties(hdf5::hdf5-shared PROPERTIES INTERFACE_COMPILE_DEFINITIONS USEHDF5) # Set as target property for compatibility with old CMake
elseif(HDF5_FOUND)
  # Repackage results from FindHDF5 Module as target
  message(STATUS "Building with HDF5 support")
  if (NOT TARGET module_hdf5-shared)
    add_library(module_hdf5-shared INTERFACE)
    add_library(module_hdf5_cpp-shared INTERFACE)
    set_target_properties(module_hdf5-shared PROPERTIES
      INTERFACE_LINK_LIBRARIES "${HDF5_C_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${HDF5_C_INCLUDE_DIRS}"
      INTERFACE_COMPILE_DEFINITIONS "${HDF5_C_DEFINITIONS}")
    add_library(hdf5::hdf5-shared ALIAS module_hdf5-shared)
    set_target_properties(module_hdf5_cpp-shared PROPERTIES
      INTERFACE_LINK_LIBRARIES "${HDF5_CXX_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${HDF5_CXX_INCLUDE_DIRS}"
      INTERFACE_COMPILE_DEFINITIONS "${HDF5_CXX_DEFINITIONS}")
    #target_compile_definitions(module_hdf5_cpp-shared INTERFACE USEHDF5)
    set_target_properties(module_hdf5_cpp-shared PROPERTIES INTERFACE_COMPILE_DEFINITIONS USEHDF5) # Set as target property for compatibility with old CMake
    add_library(hdf5::hdf5_cpp-shared ALIAS module_hdf5_cpp-shared)
  endif()
else()
  message(WARNING "Building without HDF5 support")
endif()

if (EXISTS ${local_hdf5_dir})
  if(WIN32)
    file(GLOB lib_files ${local_hdf5_dir}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}hdf5${CMAKE_SHARED_LIBRARY_SUFFIX}
                        ${local_hdf5_dir}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}hdf5_cpp${CMAKE_SHARED_LIBRARY_SUFFIX})
    install(FILES ${lib_files} DESTINATION bin)
  else()
    file(GLOB lib_files ${local_hdf5_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}hdf5${CMAKE_SHARED_LIBRARY_SUFFIX}*
                        ${local_hdf5_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}hdf5_cpp${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${lib_files} DESTINATION lib)
  endif()
endif()
