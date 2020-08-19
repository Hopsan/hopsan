set(fmi_home ${CMAKE_CURRENT_LIST_DIR}/fmilibrary)

add_library(fmilibrary SHARED IMPORTED)
set_target_properties(fmilibrary PROPERTIES
    IMPORTED_LOCATION ${fmi_home}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}fmilib_shared${CMAKE_SHARED_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${fmi_home}/include)
if(WIN32)
  set_target_properties(fmilibrary PROPERTIES
    IMPORTED_IMPLIB ${fmi_home}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}fmilib_shared${CMAKE_IMPORT_LIBRARY_SUFFIX})
endif()

if (EXISTS ${fmi_home})
  file(GLOB lib_files ${fmi_home}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}fmilib_shared${CMAKE_SHARED_LIBRARY_SUFFIX}*)
  install(FILES ${lib_files} DESTINATION lib)
  install(DIRECTORY ${fmi_home} DESTINATION dependencies)
endif()
