find_package(Threads REQUIRED)

add_executable(hopsanserver main.cpp)

target_include_directories(hopsanserver PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../dependencies/tclap/include>)

target_link_libraries(hopsanserver libhopsanremotecommon Threads::Threads)

set_target_properties(hopsanserver PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS hopsanserver
  RUNTIME DESTINATION bin
)
