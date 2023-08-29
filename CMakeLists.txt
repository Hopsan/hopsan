cmake_minimum_required(VERSION 3.10)
project(Hopsan)

cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)
if (POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

set(HOPSANCORE_INSTALL_DST "HopsanCore")
message(WARNING "Hopsan CMake support is still experimental, but should be  working for for GCC / MinGW and limited MSVC build")

include(CheckCXXCompilerFlag)
if (MINGW)
  check_cxx_compiler_flag("Wa,-mbig-obj" COMPILER_HAS_MBIG_OBJ)
  if (${COMPILER_HAS_MBIG_OBJ})
    set(CMAKE_CXX_FLAGS -Wa,-mbig-obj)
  endif()
endif ()

enable_testing()

add_subdirectory(doc)
add_subdirectory(HopsanCore)
add_subdirectory(componentLibraries)
add_subdirectory(hopsandcp)
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

# Install Qt and C++ runtime
if (WIN32)

  file(TO_CMAKE_PATH "$ENV{qmake_path}" qt_home_bin)
  if (EXISTS ${qt_home_bin})
      message(STATUS "Installing from QT directory: ${qt_home_bin}")

      install(FILES ${qt_home_bin}/Qt5Core.dll
                    ${qt_home_bin}/Qt5Gui.dll
                    ${qt_home_bin}/Qt5Network.dll
                    ${qt_home_bin}/Qt5OpenGL.dll
                    ${qt_home_bin}/Qt5Widgets.dll
                    ${qt_home_bin}/Qt5Sql.dll
                    ${qt_home_bin}/Qt5Svg.dll
                    ${qt_home_bin}/Qt5Xml.dll
                    ${qt_home_bin}/Qt5Test.dll
                    ${qt_home_bin}/Qt5PrintSupport.dll
              DESTINATION bin)

      install(FILES ${qt_home_bin}/../plugins/iconengines/qsvgicon.dll
              DESTINATION bin/iconengines)
      install(FILES ${qt_home_bin}/../plugins/imageformats/qjpeg.dll
                    ${qt_home_bin}/../plugins/imageformats/qsvg.dll
              DESTINATION bin/imageformats)
      install(FILES ${qt_home_bin}/../plugins/platforms/qwindows.dll
              DESTINATION bin/platforms)
  else()
      message(WARNING "Env variable qmake_path not set, not installing qt binaries")
  endif()

  if (MINGW)
    file(TO_CMAKE_PATH "$ENV{mingw_path}" mingw_home_bin)
    if (EXISTS ${mingw_path})
        message(STATUS "Installing from MinGW directory: ${mingw_home_bin}")

        install(FILES ${mingw_home_bin}/libgcc_s_seh-1.dll
                      ${mingw_home_bin}/libstdc++-6.dll
                      ${mingw_home_bin}/libwinpthread-1.dll
                DESTINATION bin)

        # Qt binaries that are only part of the mingw build
        if (EXISTS ${qt_home_bin})
            install(FILES ${qt_home_bin}/Qt5WebKit.dll
                          ${qt_home_bin}/Qt5WebKitWidgets.dll
                          ${qt_home_bin}/libicuin56.dll
                          ${qt_home_bin}/libicuuc56.dll
                          ${qt_home_bin}/libicudt56.dll
                          ${qt_home_bin}/libeay32.dll
                          ${qt_home_bin}/ssleay32.dll
                    DESTINATION bin)
        endif()
    else()
        message(WARNING "Env variable mingw_path not set, not installing mingw binaries")
    endif()
  endif()

endif()
