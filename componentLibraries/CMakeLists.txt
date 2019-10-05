cmake_minimum_required(VERSION 3.0)
project(ComponentLibraries)

file(GLOB subdirs RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
foreach(subdir ${subdirs})
    if(IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${subdir} AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/${subdir}/CMakeLists.txt)
      add_subdirectory(${subdir})
    endif()
endforeach()
