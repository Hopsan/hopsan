find_package(Threads REQUIRED)

file(GLOB_RECURSE srcfiles src/*.cpp include/*.h include/*.hpp)

add_library(libhopsanremoteclient STATIC ${srcfiles})
set_target_properties(libhopsanremoteclient PROPERTIES OUTPUT_NAME hopsanremoteclient)

target_include_directories(libhopsanremoteclient PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(libhopsanremoteclient libhopsanremotecommon Threads::Threads)

set_target_properties(libhopsanremoteclient PROPERTIES INSTALL_RPATH "\$ORIGIN")

install(TARGETS libhopsanremoteclient
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include )
