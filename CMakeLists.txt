cmake_minimum_required(VERSION 3.0)
project(Hopsan)

set(HOPSANCORE_INSTALL_DST "HopsanCore")
message(WARNING "Hopsan CMake support is still experimental. Only the core libraries are build, no generator lib, GUI or network applications")

enable_testing()

add_subdirectory(HopsanCore)
add_subdirectory(componentLibraries)
add_subdirectory(HopsanCLI)
add_subdirectory(HopsanGenerator)
add_subdirectory(hopsanremote)
add_subdirectory(Ops)
add_subdirectory(SymHop)
add_subdirectory(UnitTests)
add_subdirectory(hopsanc)
