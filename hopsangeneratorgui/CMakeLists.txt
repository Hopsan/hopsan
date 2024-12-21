project(HopsanGeneratorGUI)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)

set(target_name hopsangeneratorgui)

# Find source code
file(GLOB_RECURSE srcfiles src/*.cpp)
file(GLOB_RECURSE headerfiles include/*.h include/*.hpp)

# Create library target and add source code files
add_library(${target_name} STATIC ${srcfiles} ${headerfiles})

# Set include directories
target_include_directories(${target_name} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:include>)

# Set link dependencies
target_link_libraries(${target_name} Qt5::Core Qt5::Gui Qt5::Widgets)

# Set compiler definitions
# TODO: Should create aseparat target for debug/release flag, right now this is set on hopsancore, but only those who depend on it will get the flag automatically
if(CMAKE_BUILD_TYPE MATCHES Debug)
  target_compile_definitions(${target_name} PRIVATE HOPSAN_BUILD_TYPE_DEBUG)
else()
  target_compile_definitions(${target_name} PRIVATE HOPSAN_BUILD_TYPE_RELEASE)
  target_compile_definitions(${target_name} PRIVATE QT_NO_DEBUG_OUTPUT)
endif()
