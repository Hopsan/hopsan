set(local_libzip_dir ${CMAKE_CURRENT_LIST_DIR}/libzip)
if(WIN32)
    set(CMAKE_FIND_LIBRARY_PREFIXES lib)
endif(WIN32)
find_library(libzip NAMES ${CMAKE_FIND_LIBRARY_PREFIXES}zip ${CMAKE_FIND_LIBRARY_PREFIXES}zip${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_STATIC_LIBRARY_PREFIX}zip${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS ${local_libzip_dir}/lib NO_DEFAULT_PATH)

if (libzip)
    message(STATUS "Found local libzip")
    add_library(libzip STATIC IMPORTED)
    if (MSVC)
        set_target_properties(libzip PROPERTIES
                                    IMPORTED_LOCATION ${local_libzip_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zip${CMAKE_STATIC_LIBRARY_SUFFIX}
                                    INTERFACE_INCLUDE_DIRECTORIES ${local_libzip_dir}/include)
    elseif (MINGW)
        set_target_properties(libzip PROPERTIES
                                    IMPORTED_LOCATION ${local_libzip_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zip${CMAKE_SHARED_LIBRARY_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}
                                    INTERFACE_INCLUDE_DIRECTORIES ${local_libzip_dir}/include)
    else()
        set_target_properties(libzip PROPERTIES
                                    IMPORTED_LOCATION ${local_libzip_dir}/lib/libzip.so
                                    INTERFACE_INCLUDE_DIRECTORIES ${local_libzip_dir}/include)
    endif()

else()
    message(STATUS "Looking for libzip in system")
    find_package(libzip CONFIG) # Search in system
    if (libzip_FOUND)
      message(STATUS "Found libzip")
      add_library(libzip STATIC IMPORTED)
    else()
      message(WARNING "Could not find libzip")
    endif()
endif()
