set(indexingcsvparser_dir ${CMAKE_CURRENT_LIST_DIR}/indexingcsvparser)

function(add_indexingcsvparser_src target)
  target_sources(${target} PRIVATE
    ${indexingcsvparser_dir}/src/indexingcsvparser.cpp
    ${indexingcsvparser_dir}/include/indexingcsvparser/indexingcsvparser.h)

    target_include_directories(${target} PRIVATE
      $<BUILD_INTERFACE:${indexingcsvparser_dir}/include>)
endfunction()

function(install_indexingcsvparser_src destination)
    install(DIRECTORY ${indexingcsvparser_dir}/src DESTINATION ${destination} )
    install(DIRECTORY ${indexingcsvparser_dir}/include DESTINATION ${destination} )
endfunction()
