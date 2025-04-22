set(local_ssp4c_dir ${CMAKE_CURRENT_LIST_DIR}/ssp4c)

if (EXISTS ${local_ssp4c_dir})

  set(zlibstatic_name zlibstatic)
  if (NOT WIN32)
    set(zlibstatic_name z)
  endif()

  set(ssp4c_dbg_ext "")
  if (WIN32)
    set(ssp4c_dbg_ext d)
  endif()

  set(static_ssp4c ${local_ssp4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ssp4c${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(static_ssp4c_d ${local_ssp4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ssp4c${ssp4c_dbg_ext}${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(static_zlib ${local_ssp4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${zlibstatic_name}${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(static_zlib_d ${local_ssp4c_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${zlibstatic_name}${ssp4c_dbg_ext}${CMAKE_STATIC_LIBRARY_SUFFIX})

  add_library(ssp4c STATIC IMPORTED)

  set_target_properties(ssp4c PROPERTIES
                              IMPORTED_LOCATION ${static_ssp4c}
                              IMPORTED_LOCATION_DEBUG ${static_ssp4c_d}
                              INTERFACE_INCLUDE_DIRECTORIES ${local_ssp4c_dir}/include
                              INTERFACE_COMPILE_DEFINITIONS "USESSP4C;SSP4C_STATIC")

  if (EXISTS ${static_zlib})
      message(STATUS "Found static zlib in ${local_ssp4c_dir}")
      add_library(ssp4c_zlib STATIC IMPORTED)
      set_target_properties(ssp4c_zlib PROPERTIES
                                       IMPORTED_LOCATION ${static_zlib}
                                       IMPORTED_LOCATION_DEBUG ${static_zlib_d})
      target_link_libraries(ssp4c INTERFACE ssp4c_zlib)
  else()
      find_package(ZLIB MODULE REQUIRED)
      target_link_libraries(ssp4c INTERFACE ZLIB::ZLIB)
  endif()

  install(DIRECTORY ${local_ssp4c_dir} DESTINATION dependencies)
  message(STATUS "Building with ssp4c support")
else()
  message(WARNING "Building without ssp4c support")
endif()
