project(Ops)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

file(GLOB_RECURSE ops_srcfiles src/*.cpp )

add_library(ops SHARED ${ops_srcfiles})
target_include_directories(ops PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

if(WIN32)
  target_compile_definitions(ops PRIVATE OPS_DLLEXPORT)
  target_compile_definitions(ops INTERFACE OPS_DLLIMPORT)
endif()

install(TARGETS ops
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include )
