function(copy_file_after_build targetname src_file dst_directory)
    add_custom_command(
        TARGET ${targetname}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_directory}"
        COMMAND ${CMAKE_COMMAND} -E copy "${src_file}" "${dst_directory}"
    )
endfunction()

function(copy_dir_after_build targetname src_directory dst_directory)
    get_filename_component(dirname ${src_directory} NAME)
    add_custom_command(
        TARGET ${targetname}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_directory}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${src_directory}" "${dst_directory}/${dirname}"
    )
endfunction()

function(add_cmake_subdirectories root_dir)
    file(GLOB subdirs RELATIVE ${root_dir} ${root_dir}/*)
    foreach(subdir ${subdirs})
        if(IS_DIRECTORY ${root_dir}/${subdir} AND EXISTS ${root_dir}/${subdir}/CMakeLists.txt)
          add_subdirectory(${subdir})
        endif()
    endforeach()
endfunction()

function(target_link_optional_libraries target_name)
    foreach(link_target ${ARGN})
        if(TARGET ${link_target})
            target_link_libraries(${target_name} ${link_target})
        endif()
    endforeach()
endfunction()
