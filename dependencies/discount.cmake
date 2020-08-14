set(local_discount_dir ${CMAKE_CURRENT_LIST_DIR}/discount)
add_library(discount SHARED IMPORTED)
set_target_properties(discount PROPERTIES
  IMPORTED_LOCATION ${local_discount_dir}/lib/libmarkdown.so
  INTERFACE_INCLUDE_DIRECTORIES ${local_discount_dir}/include
  INTERFACE_COMPILE_DEFINITIONS USEDISCOUNT)

if (EXISTS ${local_discount_dir})
  file(GLOB lib_files ${local_discount_dir}/lib/libmarkdown.so*)
  install(FILES ${lib_files} DESTINATION lib)
endif()
