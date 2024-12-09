find_package(Threads REQUIRED)

add_executable(hopsanservermonitor main.cpp)

target_include_directories(hopsanservermonitor PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/tclap/include>)

target_link_libraries(hopsanservermonitor libhopsanremoteclient libhopsanremotecommon Threads::Threads)

set_target_properties(hopsanservermonitor PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS hopsanservermonitor
  RUNTIME DESTINATION bin
)
