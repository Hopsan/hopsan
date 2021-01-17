set(local_discount_dir ${CMAKE_CURRENT_LIST_DIR}/discount)

if (EXISTS ${local_discount_dir})
  # Add actual discount library
  add_library(discount SHARED IMPORTED)
  if(WIN32)

    # When building with mingw, discount uses the lib prefix on windows
    set(slp ${CMAKE_SHARED_LIBRARY_PREFIX})
    if (MINGW)
      set(slp lib)
    endif()

    set_target_properties(discount PROPERTIES
    IMPORTED_LOCATION ${local_discount_dir}/bin/${slp}markdown${CMAKE_SHARED_LIBRARY_SUFFIX}
    IMPORTED_IMPLIB ${local_discount_dir}/lib/${slp}markdown${CMAKE_IMPORT_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${local_discount_dir}/include
    INTERFACE_COMPILE_DEFINITIONS USEDISCOUNT)

    # Add discount to installation
    file(GLOB lib_files ${local_discount_dir}/bin/${slp}markdown${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${lib_files} DESTINATION bin)

  else()

    set_target_properties(discount PROPERTIES
    IMPORTED_LOCATION ${local_discount_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}markdown${CMAKE_SHARED_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${local_discount_dir}/include
    INTERFACE_COMPILE_DEFINITIONS USEDISCOUNT)

    # Add discount to installation
    file(GLOB lib_files ${local_discount_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}markdown${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${lib_files} DESTINATION lib)

  endif()
else()
  message(WARNING "Building without Discount (libmarkdown) support")
endif()
