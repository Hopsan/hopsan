set(local_xerces_dir ${CMAKE_CURRENT_LIST_DIR}/xerces)
set(local_zlib_dir ${CMAKE_CURRENT_LIST_DIR}/zlib)
set(local_libzip_dir ${CMAKE_CURRENT_LIST_DIR}/libzip)
set(local_asio_dir ${CMAKE_CURRENT_LIST_DIR}/asio-code)
set(local_dcplib_dir ${CMAKE_CURRENT_LIST_DIR}/dcplib)


find_package(Threads)
find_package(ICU MODULE COMPONENTS uc data)
find_package(XercesC CONFIG PATHS ${local_xerces_dir} NO_DEFAULT_PATH)
find_package(libzip CONFIG PATHS ${local_libzip_dir} NO_DEFAULT_PATH)

#add_library(ZIP::ZIP ALIAS libzip::zip)
# add alias compatible with older cmake versions
add_library(ZIP::ZIP INTERFACE IMPORTED)
set_target_properties(ZIP::ZIP PROPERTIES INTERFACE_LINK_LIBRARIES libzip::zip)

# TODO Use proper ASIO lookup and installation
add_library(ASIO::ASIO INTERFACE IMPORTED)
set_target_properties(ASIO::ASIO PROPERTIES
    INTERFACE_COMPILE_DEFINITIONS "ASIO_STANDALONE"
    INTERFACE_INCLUDE_DIRECTORIES "${local_asio_dir}/include"
)
target_link_libraries(ASIO::ASIO INTERFACE Threads::Threads)

find_package(DCPLib CONFIG PATHS ${local_dcplib_dir} NO_DEFAULT_PATH)


# Add zlib to installation if local variant found
if (EXISTS ${local_zlib_dir})
  # When building with mingw, zlib uses the lib prefix on windows
  if (MINGW)
    file(GLOB lib_files ${local_zlib_dir}/bin/libzlib${CMAKE_SHARED_LIBRARY_SUFFIX})
  else()
    file(GLOB lib_files ${local_zlib_dir}/bin/zlib${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()
  install(FILES ${lib_files} DESTINATION bin)
endif()

# Add libzip to installation if local variant found
if (EXISTS ${local_libzip_dir})
  # When building with mingw, libzip uses the lib prefix on windows
  if (WIN32)
    if (MINGW)
      file(GLOB lib_files ${local_libzip_dir}/bin/libzip${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
      file(GLOB lib_files ${local_libzip_dir}/bin/zip*${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
    install(FILES ${lib_files} DESTINATION bin)
  else()
    file(GLOB lib_files ${local_libzip_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}zip${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${lib_files} DESTINATION lib)
  endif()
endif()

# Add xerces to installation if local variant found
if (EXISTS ${local_xerces_dir})
  # When building with mingw, xerces uses the lib prefix on windows
  if (WIN32)
    if (MINGW)
      file(GLOB lib_files ${local_xerces_dir}/bin/libxerces-c${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
      file(GLOB lib_files ${local_xerces_dir}/bin/xerces-c*${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
    install(FILES ${lib_files} DESTINATION bin)
  else()
    file(GLOB lib_files ${local_xerces_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}xerces-c*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${lib_files} DESTINATION lib)
  endif()
endif()
