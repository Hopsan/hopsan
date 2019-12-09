set(fmi_home ${CMAKE_CURRENT_LIST_DIR}/FMILibrary)

add_library(fmilibrary SHARED IMPORTED)
set_target_properties(fmilibrary PROPERTIES
    IMPORTED_LOCATION ${fmi_home}/lib/libfmilib_shared${CMAKE_SHARED_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${fmi_home}/include)
if(WIN32)
  set_target_properties(fmilibrary PROPERTIES
    IMPORTED_IMPLIB ${fmi_home}/lib/libfmilib_shared${CMAKE_IMPORT_LIBRARY_SUFFIX})
endif()
