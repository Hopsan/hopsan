set(local_fmi4c_dir ${CMAKE_CURRENT_LIST_DIR}/fmi4c)

if (EXISTS ${local_fmi4c_dir})
  # Add fmi4c library
  add_library(fmi4c SHARED IMPORTED)
  if(WIN32)

    set_target_properties(fmi4c PROPERTIES
    IMPORTED_LOCATION ${local_fmi4c_dir}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}fmi4c${CMAKE_SHARED_LIBRARY_SUFFIX}
    IMPORTED_IMPLIB ${local_fmi4c_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}fmi4c${CMAKE_IMPORT_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${local_fmi4c_dir}/include
    INTERFACE_COMPILE_DEFINITIONS USEFMI4C)

    # Add fmi4c to installation
    install(FILES ${local_fmi4c_dir}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}fmi4c${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION bin)

  else()

    set_target_properties(fmi4c PROPERTIES
    IMPORTED_LOCATION ${local_fmi4c_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}fmi4c${CMAKE_SHARED_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${local_fmi4c_dir}/include
    INTERFACE_COMPILE_DEFINITIONS USEFMI4C)

    # Add fmi4c to installation
    file(GLOB lib_files ${local_fmi4c_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}fmi4c${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${lib_files} DESTINATION lib)

  endif()
  install(DIRECTORY ${local_fmi4c_dir} DESTINATION dependencies)
  message(STATUS "Building with Fmi4c support")
else()
  message(WARNING "Building without Fmi4c support")
endif()
