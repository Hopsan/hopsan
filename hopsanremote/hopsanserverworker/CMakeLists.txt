find_package(Threads REQUIRED)

add_executable(hopsanserverworker main.cpp)

target_include_directories(hopsanserverworker PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../dependencies/tclap/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../Utilities>)

target_link_libraries(hopsanserverworker libhopsanremotecommon hopsancore Threads::Threads)

set_target_properties(hopsanserverworker PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(TARGETS hopsanserverworker
  RUNTIME DESTINATION bin
)
