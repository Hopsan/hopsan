cmake_minimum_required(VERSION 3.0)
project(HopsanCore)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)
find_package(Threads)

set(HOPSANCORE_INSTALL_DST "" CACHE STRING "Hopsan source code installation relative CMAKE_INSTALL_PREFIX ")

file(GLOB_RECURSE HopsanCore_srcfiles src/*.cpp)
file(GLOB_RECURSE HopsanCore_incfiles include/*.h include/*.hpp)

file(GLOB_RECURSE numhop_srcfiles dependencies/libnumhop/src/*.cpp)
file(GLOB_RECURSE csvparser_srcfiles dependencies/indexingcsvparser/src/indexingcsvparser.cpp)

add_library(HopsanCore SHARED ${HopsanCore_srcfiles}
                              ${numhop_srcfiles}
                              ${csvparser_srcfiles})

if(WIN32)
  target_compile_definitions(HopsanCore PRIVATE HOPSANCORE_DLLEXPORT)
endif()
target_compile_definitions(HopsanCore PRIVATE HOPSANCORE_WRITELOG)

target_include_directories(HopsanCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_include_directories(HopsanCore PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libnumhop/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dependencies/indexingcsvparser/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dependencies/rapidxml>)

target_link_libraries(HopsanCore Threads::Threads ${CMAKE_DL_LIBS})

install(TARGETS HopsanCore
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

if (NOT ${HOPSANCORE_INSTALL_DST} STREQUAL "")
  set(HOPSANCORE_INSTALL_DST ${HOPSANCORE_INSTALL_DST}/)
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${HOPSANCORE_INSTALL_DST}include )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ DESTINATION ${HOPSANCORE_INSTALL_DST}src )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libnumhop/include/ DESTINATION ${HOPSANCORE_INSTALL_DST}dependencies/libnumhop/include )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libnumhop/src/ DESTINATION ${HOPSANCORE_INSTALL_DST}dependencies/libnumhop/src )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/indexingcsvparser/ DESTINATION ${HOPSANCORE_INSTALL_DST}dependencies/indexingcsvparser )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/rapidxml/ DESTINATION ${HOPSANCORE_INSTALL_DST}dependencies/rapidxml )
