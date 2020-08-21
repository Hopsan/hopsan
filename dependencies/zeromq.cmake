set(local_zeromq_dir ${CMAKE_CURRENT_LIST_DIR}/zeromq)
find_package(ZeroMQ CONFIG REQUIRED PATHS ${local_zeromq_dir} NO_DEFAULT_PATH) # Search for local version
find_package(ZeroMQ CONFIG REQUIRED) # Search in system

if (ZeroMQ_FOUND)
  message(STATUS "Building with ZeroMQ support")
  target_compile_definitions(libzmq INTERFACE USEZMQ)
else()
  message(WARNING "Building without ZeroMQ support")
endif()

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
