set(local_fmi4c_dir ${CMAKE_CURRENT_LIST_DIR}/fmi4c)

if (EXISTS ${local_fmi4c_dir})

  set(zlibstatic_name zlibstatic)
  if (NOT WIN32)
    set(zlibstatic_name z)
  endif()

  set(fmi4c_dbg_ext "")
  if (WIN32)
    set(fmi4c_dbg_ext d)
  endif()

  set(static_fmi4c ${local_fmi4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}fmi4c${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(static_fmi4c_d ${local_fmi4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}fmi4c${fmi4c_dbg_ext}${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(static_zlib ${local_fmi4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${zlibstatic_name}${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(static_zlib_d ${local_fmi4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${zlibstatic_name}${fmi4c_dbg_ext}${CMAKE_STATIC_LIBRARY_SUFFIX})

  add_library(fmi4c STATIC IMPORTED)

  set_target_properties(fmi4c PROPERTIES
                              IMPORTED_LOCATION ${static_fmi4c}
                              IMPORTED_LOCATION_DEBUG ${static_fmi4c_d}
                              INTERFACE_INCLUDE_DIRECTORIES ${local_fmi4c_dir}/include
                              INTERFACE_COMPILE_DEFINITIONS "USEFMI4C;FMI4C_STATIC")

  if (EXISTS ${static_zlib})
      message(STATUS "Found static zlib in ${local_fmi4c_dir}")
      add_library(fmi4c_zlib STATIC IMPORTED)
      set_target_properties(fmi4c_zlib PROPERTIES
                                       IMPORTED_LOCATION ${static_zlib}
                                       IMPORTED_LOCATION_DEBUG ${static_zlib_d})
      #target_link_libraries(fmi4c INTERFACE fmi4c_zlib)
      set_target_properties(fmi4c PROPERTIES INTERFACE_LINK_LIBRARIES fmi4c_zlib) # Compatible with old cmake 3.10
  else()
      find_package(ZLIB MODULE REQUIRED)
      #target_link_libraries(fmi4c INTERFACE ZLIB::ZLIB)
      set_target_properties(fmi4c PROPERTIES INTERFACE_LINK_LIBRARIES ZLIB::ZLIB) # Compatible with old cmake 3.10
  endif()

  install(DIRECTORY ${local_fmi4c_dir} DESTINATION dependencies)
  message(STATUS "Building with Fmi4c support")
else()
  message(WARNING "Building without Fmi4c support")
endif()
