cmake_minimum_required(VERSION 3.0)
project(Hopsan)

set(HOPSANCORE_INSTALL_DST "HopsanCore")
message(WARNING "Hopsan CMake support is still experimental. Only the core libraries are build, no generator lib, GUI or network applications")

add_subdirectory(HopsanCore)
add_subdirectory(componentLibraries)
add_subdirectory(HopsanCLI)
#add_subdirectory(HopsanGenerator)
add_subdirectory(Ops)
