set(local_xerces_dir ${CMAKE_CURRENT_LIST_DIR}/xerces)
set(local_libzip_dir ${CMAKE_CURRENT_LIST_DIR}/libzip)
set(local_asio_dir ${CMAKE_CURRENT_LIST_DIR}/asio-code)
set(local_dcplib_dir ${CMAKE_CURRENT_LIST_DIR}/dcplib)


find_package(Threads)
find_package(ICU MODULE COMPONENTS uc data)
find_package(XercesC CONFIG PATHS ${local_xerces_dir} NO_DEFAULT_PATH)
find_package(libzip CONFIG PATHS ${local_libzip_dir} NO_DEFAULT_PATH)
add_library(ZIP::ZIP ALIAS libzip::zip)
# TODO Use proper ASIO lookup and installation
add_library(ASIO::ASIO INTERFACE IMPORTED)
set_target_properties(ASIO::ASIO PROPERTIES
    INTERFACE_COMPILE_DEFINITIONS "ASIO_STANDALONE"
    INTERFACE_INCLUDE_DIRECTORIES "${local_asio_dir}/include"
)
target_link_libraries(ASIO::ASIO INTERFACE Threads::Threads)

find_package(DCPLib CONFIG PATHS ${local_dcplib_dir} NO_DEFAULT_PATH)
