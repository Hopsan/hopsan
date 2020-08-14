set(local_qwt_dir ${CMAKE_CURRENT_LIST_DIR}/qwt)

add_library(qwt SHARED IMPORTED)
set_target_properties(qwt PROPERTIES
  IMPORTED_LOCATION ${local_qwt_dir}/lib/libqwt.so
  INTERFACE_INCLUDE_DIRECTORIES ${local_qwt_dir}/include
  INTERFACE_COMPILE_DEFINITIONS QWT_DLL)

file(GLOB lib_files ${local_qwt_dir}/lib/libqwt.so*)
install(FILES ${lib_files} DESTINATION lib)
