set(local_msgpack_dir ${CMAKE_CURRENT_LIST_DIR}/msgpack-c)
find_package(msgpack CONFIG PATHS ${local_msgpack_dir} NO_DEFAULT_PATH) # Search for local version
if (NOT msgpack_FOUND)
  message(STATUS "Looking for msgpack in system")
  find_package(msgpack CONFIG) # Search in system
  if (msgpack_FOUND)
    message(STATUS "  Found msgpack")
  else()
    # Add dummy library
    if (NOT TARGET msgpackc-cxx)
      add_library(msgpackc-cxx INTERFACE)
    endif()
  endif()
endif()
