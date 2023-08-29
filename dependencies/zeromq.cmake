set(local_zeromq_dir ${CMAKE_CURRENT_LIST_DIR}/zeromq)
find_package(ZeroMQ CONFIG PATHS ${local_zeromq_dir} NO_DEFAULT_PATH) # Search for local version
if (NOT WIN32)
  find_package(PkgConfig QUIET)
endif()

# Try using PkgConfig to find installation in system
if (NOT ZeroMQ_FOUND AND PKG_CONFIG_FOUND)
  message(STATUS "Looking for ZeroMQ using PkgConfig")
  pkg_check_modules(zmq IMPORTED_TARGET libzmq)
endif()

if (ZeroMQ_FOUND)
  message(STATUS "Building with ZeroMQ support")
  #target_compile_definitions(libzmq INTERFACE USEZMQ)
  set_target_properties(libzmq PROPERTIES INTERFACE_COMPILE_DEFINITIONS USEZMQ) # Set as target property for compatibility with old CMake
elseif(zmq_FOUND)
  message(STATUS "Building with ZeroMQ support (found by PkgConfig)")
  if (NOT TARGET libzmq)
    add_library(libzmq INTERFACE)
  endif()
  #target_link_libraries(libzmq INTERFACE PkgConfig::zmq)
  #target_compile_definitions(libzmq INTERFACE USEZMQ)
  set_target_properties(libzmq PROPERTIES INTERFACE_LINK_LIBRARIES PkgConfig::zmq
                                          INTERFACE_COMPILE_DEFINITIONS USEZMQ) # Set as target property for compatibility with old CMake
else()
  message(WARNING "Building without ZeroMQ support")
endif()

# Add ZeroMQ to installation if local variant found
if (EXISTS ${local_zeromq_dir})
  # When building with mingw, zeromq uses the lib prefix on windows
  if (WIN32)
    if (MINGW)
      file(GLOB lib_files ${local_zeromq_dir}/bin/libzmq${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
      file(GLOB lib_files ${local_zeromq_dir}/bin/libzmq*${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
    install(FILES ${lib_files} DESTINATION bin)
  else()
    file(GLOB lib_files ${local_zeromq_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}zmq${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${lib_files} DESTINATION lib)
  endif()
endif()
