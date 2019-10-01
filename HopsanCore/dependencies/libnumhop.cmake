set(libnumhop_dir ${CMAKE_CURRENT_LIST_DIR}/libnumhop)

function(add_libnumhop_src target)
  file(GLOB_RECURSE numhop_cpp ${libnumhop_dir}/src/*.cpp)
  file(GLOB_RECURSE numhop_headers ${libnumhop_dir}/include/*.h)
  target_sources(${target} PRIVATE ${numhop_cpp} ${numhop_headers})

  target_include_directories(${target} PRIVATE
    $<BUILD_INTERFACE:${libnumhop_dir}/include>)
endfunction()

function(install_libnumhop_src destination)
    install(DIRECTORY ${libnumhop_dir}/src DESTINATION ${destination} )
    install(DIRECTORY ${libnumhop_dir}/include DESTINATION ${destination} )
endfunction()
