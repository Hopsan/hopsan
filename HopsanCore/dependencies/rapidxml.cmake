set(rapidxml_dir ${CMAKE_CURRENT_LIST_DIR}/rapidxml)

function(add_rapidxml_src target)
  file(GLOB_RECURSE rapidxml_headers ${rapidxml_dir}/*.hpp)
  target_sources(${target} PRIVATE ${rapidxml_headers})

  target_include_directories(${target} PRIVATE
    $<BUILD_INTERFACE:${rapidxml_dir}>)
endfunction()

function(install_rapidxml_src destination)
    install(DIRECTORY ${rapidxml_dir} DESTINATION ${destination} )
endfunction()
