include(${CMAKE_CURRENT_LIST_DIR}/../../dependencies/zeromq.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../dependencies/msgpack.cmake)

file(GLOB_RECURSE srcfiles src/*.cpp include/*.h include/*.hpp)

add_library(libhopsanremotecommon STATIC ${srcfiles})
set_target_properties(libhopsanremotecommon PROPERTIES OUTPUT_NAME hopsanremotecommon)

target_include_directories(libhopsanremotecommon PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(libhopsanremotecommon PUBLIC libzmq msgpackc-cxx)

install(TARGETS libhopsanremotecommon
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include )
