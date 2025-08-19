project(HopsanCore)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX _d)
if(WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()
find_package(Threads)

set(HOPSANCORE_INSTALL_DST "" CACHE STRING "Hopsan source code installation relative CMAKE_INSTALL_PREFIX ")

# Include dependencies functions
include(dependencies/libnumhop.cmake)
include(dependencies/indexingcsvparser.cmake)
include(dependencies/rapidxml.cmake)
include(dependencies/sundials.cmake)

# Find HopsanCore source code
file(GLOB_RECURSE hopsancore_srcfiles src/*.cpp)
file(GLOB_RECURSE hopsancore_headerfiles include/*.h include/*.hpp)

# Create HospanCore library target and add source code files
add_library(hopsancore SHARED ${hopsancore_srcfiles} ${hopsancore_headerfiles})
add_libnumhop_src(hopsancore)
add_indexingcsvparser_src(hopsancore)
add_rapidxml_src(hopsancore)
add_sundials_src(hopsancore)

# Set compiler definitions
if(WIN32)
  target_compile_definitions(hopsancore PRIVATE HOPSANCORE_DLLEXPORT)
  target_compile_definitions(hopsancore INTERFACE HOPSANCORE_DLLIMPORT)
endif()
if (MSVC)
  # Ensure that the __cplusplus macro is set to the appropriate value in MSVC
  # otherwise it is stuck at 199711L
  target_compile_options(hopsancore PUBLIC "/Zc:__cplusplus")
  # Undefine odd min and max macros to enable use of std::min, std::max, make it public so all projects get this define
  target_compile_definitions(hopsancore PUBLIC NOMINMAX)
endif()
target_compile_definitions(hopsancore PRIVATE HOPSANCORE_WRITELOG)
if(CMAKE_BUILD_TYPE MATCHES Debug)
  target_compile_definitions(hopsancore PUBLIC HOPSAN_BUILD_TYPE_DEBUG)
else()
  target_compile_definitions(hopsancore PUBLIC HOPSAN_BUILD_TYPE_RELEASE)
endif()

# Set include directories
target_include_directories(hopsancore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

# Set system link dependencies
target_link_libraries(hopsancore Threads::Threads ${CMAKE_DL_LIBS})

# Set HopsanCore installation destinations
install(TARGETS hopsancore
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

if (NOT ${HOPSANCORE_INSTALL_DST} STREQUAL "")
  set(HOPSANCORE_INSTALL_DST ${HOPSANCORE_INSTALL_DST}/)
endif()

# Install HopsanCore source code
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${HOPSANCORE_INSTALL_DST}include )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ DESTINATION ${HOPSANCORE_INSTALL_DST}src )
# Install dependencies source code
install_libnumhop_src(${HOPSANCORE_INSTALL_DST}dependencies/libnumhop)
install_indexingcsvparser_src(${HOPSANCORE_INSTALL_DST}dependencies/indexingcsvparser)
install_rapidxml_src(${HOPSANCORE_INSTALL_DST}dependencies/rapidxml)
install_sundials_src(${HOPSANCORE_INSTALL_DST}dependencies/sundials)
