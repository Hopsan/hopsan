cmake_minimum_required(VERSION 3.0)
project(Hopsan)

set(HOPSANCORE_INSTALL_DST "HopsanCore")
message(WARNING "Hopsan CMake support is still experimental. Only the core libraries are build, no generator lib, GUI or network applications")

enable_testing()

add_subdirectory(HopsanCore)
add_subdirectory(componentLibraries)
add_subdirectory(HopsanCLI)
add_subdirectory(HopsanGUI)
add_subdirectory(HopsanGenerator)
add_subdirectory(hopsangeneratorgui)
add_subdirectory(hopsanhdf5exporter)
add_subdirectory(hopsanremote)
add_subdirectory(Ops)
add_subdirectory(SymHop)
add_subdirectory(UnitTests)
add_subdirectory(hopsanc)

# Install various files and directories
install(FILES Hopsan-release-notes.txt
              README.md
        DESTINATION .)
install(DIRECTORY Scripts
        DESTINATION .)
install(DIRECTORY dependencies/tools/7z
        DESTINATION dependencies/tools)
install(DIRECTORY "Models/Benchmark Models"
                  "Models/Component Test"
                  "Models/Example Models"
        DESTINATION Models)
