set(local_msgpack_dir ${CMAKE_CURRENT_LIST_DIR}/msgpack-c)
find_package(msgpack CONFIG REQUIRED PATHS ${local_msgpack_dir} NO_DEFAULT_PATH) # Search for local version
find_package(msgpack CONFIG REQUIRED) # Search in system
