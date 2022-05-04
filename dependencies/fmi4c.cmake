set(local_fmi4c_dir ${CMAKE_CURRENT_LIST_DIR}/fmi4c)

if (EXISTS ${local_fmi4c_dir})

  set(zlib_name zlib)
  if (NOT WIN32)
    set(zlib_name z)
  endif()

  set(static_fmi4c ${local_fmi4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}fmi4c${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(static_zlib ${local_fmi4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${zlib_name}${CMAKE_STATIC_LIBRARY_SUFFIX})

  add_library(fmi4c STATIC IMPORTED)

  set_target_properties(fmi4c PROPERTIES
                              IMPORTED_LOCATION ${static_fmi4c}
                              INTERFACE_INCLUDE_DIRECTORIES ${local_fmi4c_dir}/include
                              INTERFACE_COMPILE_DEFINITIONS "USEFMI4C;FMI4C_STATIC")

  if (EXISTS ${static_zlib})
      message(STATUS "Found static zlib in Fmi4c")
      add_library(fmi4c_zlib STATIC IMPORTED)
      set_target_properties(fmi4c_zlib PROPERTIES
                                       IMPORTED_LOCATION ${static_zlib})
      target_link_libraries(fmi4c INTERFACE fmi4c_zlib)
  endif()

  install(DIRECTORY ${local_fmi4c_dir} DESTINATION dependencies)
  message(STATUS "Building with Fmi4c support")
else()
  message(WARNING "Building without Fmi4c support")
endif()
