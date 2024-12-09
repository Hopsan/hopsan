find_package(Threads REQUIRED)

file(GLOB srcfiles *.cpp *.h)

add_executable(hopsanaddresserver ${srcfiles})

target_include_directories(hopsanaddresserver PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/tclap/include>)

target_link_libraries(hopsanaddresserver libhopsanremoteclient Threads::Threads)

set_target_properties(hopsanaddresserver PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS hopsanaddresserver
  RUNTIME DESTINATION bin
)
