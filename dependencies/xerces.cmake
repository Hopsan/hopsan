if (NOT TARGET xercesc)
    set(local_xerces_dir ${CMAKE_CURRENT_LIST_DIR}/xerces)
    set(CMAKE_FIND_LIBRARY_PREFIXES lib)
    find_library(xercesc NAMES xerces-c ${CMAKE_STATIC_LIBRARY_PREFIX}xerces-c_3D${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS ${local_xerces_dir}/lib)

    #set(CMAKE_FIND_DEBUG_MODE TRUE)
    #find_package(xercesc CONFIG PATHS ${local_xerces_dir} NO_DEFAULT_PATH REQUIRED) # Search for local version
    #set(CMAKE_FIND_DEBUG_MODE FALSE)

    if (xercesc)
        message(STATUS "Found local xerces")
        #add_library(xercesc INTERFACE)

        add_library(xercesc STATIC IMPORTED)

        if (MINGW)
            set_target_properties(xercesc PROPERTIES
                                    IMPORTED_LOCATION ${local_xerces_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}xerces-c${CMAKE_SHARED_LIBRARY_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}
                                    INTERFACE_INCLUDE_DIRECTORIES ${local_xerces_dir}/include)
        elseif(MSVC)
            set_target_properties(xercesc PROPERTIES
                                    IMPORTED_LOCATION ${local_xerces_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}xerces-c_3D${CMAKE_STATIC_LIBRARY_SUFFIX}
                                    INTERFACE_INCLUDE_DIRECTORIES ${local_xerces_dir}/include)
        else()
            set_target_properties(xercesc PROPERTIES
                                    IMPORTED_LOCATION ${local_xerces_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}xerces-c${CMAKE_SHARED_LIBRARY_SUFFIX}
                                    INTERFACE_INCLUDE_DIRECTORIES ${local_xerces_dir}/include)
        endif()

    else()
        message(STATUS "Looking for xerces in system")
        find_package(xercesc CONFIG) # Search in system
        if (xercesc_FOUND)
            message(STATUS "  Found xerces")
            add_library(xercesc INTERFACE)
        else()
            message(WARNING "Could not find xerces")
        endif()
    endif()
endif()
