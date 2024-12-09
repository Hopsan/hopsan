add_executable(hopsanremoteclient main.cpp)

target_include_directories(hopsanremoteclient PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/tclap/include>)

target_link_libraries(hopsanremoteclient libhopsanremoteclient)

set_target_properties(hopsanremoteclient PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS hopsanremoteclient
  RUNTIME DESTINATION bin
)
