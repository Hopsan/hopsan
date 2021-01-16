set(local_qwt_dir ${CMAKE_CURRENT_LIST_DIR}/qwt)
if (NOT WIN32)
  find_package(PkgConfig QUIET)
endif()

if (EXISTS ${local_qwt_dir})

  add_library(qwt SHARED IMPORTED)

  if (WIN32)

    # Qwt build with mingw has import library suffix .a, not .dll.a as expected by CMAKE_IMPORT_LIBRARY_SUFFIX
    set(ils ${CMAKE_IMPORT_LIBRARY_SUFFIX})
    if (MINGW)
      set(ils .a)
    endif()

    set_target_properties(qwt PROPERTIES
      IMPORTED_LOCATION ${local_qwt_dir}/lib/qwt${CMAKE_SHARED_LIBRARY_SUFFIX}
      IMPORTED_LOCATION_DEBUG ${local_qwt_dir}/lib/qwtd${CMAKE_SHARED_LIBRARY_SUFFIX}
      IMPORTED_IMPLIB ${local_qwt_dir}/lib/${CMAKE_IMPORT_LIBRARY_PREFIX}qwt${ils}
      IMPORTED_IMPLIB_DEBUG ${local_qwt_dir}/lib/${CMAKE_IMPORT_LIBRARY_PREFIX}qwtd${ils}
      INTERFACE_INCLUDE_DIRECTORIES ${local_qwt_dir}/include
      INTERFACE_COMPILE_DEFINITIONS QWT_DLL)

    if (EXISTS ${local_qwt_dir})
      set(ds "")
      if (${CMAKE_BUILD_TYPE} MATCHES Debug)
        set(ds "d")
      endif()
      file(GLOB lib_files ${local_qwt_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}qwt${ds}${CMAKE_SHARED_LIBRARY_SUFFIX})
      install(FILES ${lib_files} DESTINATION bin)
    endif()

  else()
    set_target_properties(qwt PROPERTIES
      IMPORTED_LOCATION ${local_qwt_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}qwt${CMAKE_SHARED_LIBRARY_SUFFIX}
      INTERFACE_INCLUDE_DIRECTORIES ${local_qwt_dir}/include
      INTERFACE_COMPILE_DEFINITIONS QWT_DLL)

    if (EXISTS ${local_qwt_dir})
      file(GLOB lib_files ${local_qwt_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}qwt${CMAKE_SHARED_LIBRARY_SUFFIX}*)
      install(FILES ${lib_files} DESTINATION lib)
    endif()

  endif()
elseif(PKG_CONFIG_FOUND)

  message(STATUS "Looking for Qwt using PkgConfig")
  pkg_check_modules(qwt IMPORTED_TARGET Qt5Qwt6)
  add_library(qwt INTERFACE)
  target_link_libraries(qwt INTERFACE PkgConfig::qwt)

endif()



