option(BUILD_USER_DOCUMENTATION "Build user documentation" OFF])
option(BUILD_DEVELOPER_DOCUMENTATION "Build developer documentation" OFF])

if (${BUILD_USER_DOCUMENTATION} OR ${BUILD_DEVELOPER_DOCUMENTATION})

  find_package(Doxygen REQUIRED dot)

  set(DOXYGEN_BUILTIN_STL_SUPPORT    YES)
  set(DOXYGEN_DISTRIBUTE_GROUP_DOC   YES)
  set(DOXYGEN_EXTRACT_STATIC         YES)
  set(DOXYGEN_CASE_SENSE_NAMES       YES)
  set(DOXYGEN_SORT_GROUP_NAMES       YES)
  set(DOXYGEN_GENERATE_TODOLIST      NO)
  set(DOXYGEN_SHOW_FILES             NO)
  set(DOXYGEN_WARN_IF_UNDOCUMENTED   NO)
  set(DOXYGEN_WARN_LOGFILE           doxygenUserWarnings.log)
  set(DOXYGEN_FILE_PATTERNS          *.cc *.cpp *.h *.hpp *.dox)
  set(DOXYGEN_EXAMPLE_PATH           ${PROJECT_SOURCE_DIR}/componentLibraries/exampleComponentLib)
  set(DOXYGEN_IMAGE_PATH             ${PROJECT_SOURCE_DIR}/doc/graphics)
  set(DOXYGEN_ALPHABETICAL_INDEX     NO)
  set(DOXYGEN_DISABLE_INDEX          YES)
  set(DOXYGEN_GENERATE_TREEVIEW      YES)
  set(DOXYGEN_GENERATE_HTML          YES)
  set(DOXYGEN_HTML_EXTRA_STYLESHEET  hopsan.css)
  set(DOXYGEN_SEARCHENGINE           NO)
  set(DOXYGEN_CALL_GRAPH             YES)
  set(DOXYGEN_CALLER_GRAPH           YES)
  set(DOXYGEN_DOT_IMAGE_FORMAT       svg)
  set(DOXYGEN_ALIASES                [[printtodo{1}="<b>ToDo:</b> <i>\1</i> \todo \1"]]
                                     [[htmllinkimage{2}="\htmlonly <br><a href=\"\1\"><img src=\"\1\" width=\2 border=0></a><br> \endhtmlonly"]]
                                     [[htmlimagerightcaption{2}="\htmlonly <table style=\"border-spacing: 20px 10px\" border=\"0\"> <tr><td><img src=\"\1\" border=0></td><td><b>\2</b></td></tr> </table> \endhtmlonly"]])
  set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES)

  set(DOXYGEN_RECURSIVE              YES)
  set(documentation_directories      ${PROJECT_SOURCE_DIR}/HopsanCore/include
                                     ${PROJECT_SOURCE_DIR}/HopsanCore/src
                                     ${PROJECT_SOURCE_DIR}/hopsanc
                                     ${PROJECT_SOURCE_DIR}/doc)
  set(DOXYGEN_EXCLUDE_PATTERNS       "*/sundials/*"
                                     "*/sundials-extra/*")

  if (${BUILD_DEVELOPER_DOCUMENTATION})
    set(DOXYGEN_EXTRACT_PRIVATE       YES)
    set(DOXYGEN_EXTRACT_LOCAL_CLASSES YES)
    set(DOXYGEN_HIDE_SCOPE_NAMES      NO)
    set(DOXYGEN_SHOW_INCLUDE_FILES    YES)
    set(DOXYGEN_SORT_BY_SCOPE_NAME    YES)
    set(DOXYGEN_GENERATE_TODOLIST     YES)
    set(DOXYGEN_ENABLED_SECTIONS      "DEVELOPERDOCUMENTATION")
    set(DOXYGEN_SHOW_USED_FILES       YES)
    set(DOXYGEN_SHOW_FILES            YES)
    set(DOXYGEN_VERBATIM_HEADERS      YES)
    set(DOXYGEN_HAVE_DOT              YES)
    set(documentation_directories     ${PROJECT_SOURCE_DIR}/HopsanCore
                                      ${PROJECT_SOURCE_DIR}/HopsanGUI
                                      ${PROJECT_SOURCE_DIR}/HopsanCLI
                                      ${PROJECT_SOURCE_DIR}/HopsanGenerator
                                      ${PROJECT_SOURCE_DIR}/SymHop
                                      ${PROJECT_SOURCE_DIR}/Ops
                                      ${PROJECT_SOURCE_DIR}/UnitTests
                                      ${PROJECT_SOURCE_DIR}/Utilities
                                      ${PROJECT_SOURCE_DIR}/hopsanc
                                      ${PROJECT_SOURCE_DIR}/componentLibraries
                                      ${PROJECT_SOURCE_DIR}/hopsangeneratorgui
                                      ${PROJECT_SOURCE_DIR}/hopsanhdf5exporter
                                      ${PROJECT_SOURCE_DIR}/hopsanremote
                                      ${PROJECT_SOURCE_DIR}/doc)
  endif()

  # Ensure that all images are copied, they are not auto copied since most of them are added through alias commands
  file(GLOB png_files ${CMAKE_CURRENT_LIST_DIR}/graphics/*.png)
  set(DOXYGEN_HTML_EXTRA_FILES ${png_files})

  # On MINGW CMake TO_NATIVE_PATH inside FindDoxygen does not work, and we need to fix the path here
  if (MINGW)
    string(REPLACE "\\" "" DOXYGEN_DOT_PATH "${DOXYGEN_DOT_PATH}")
    string(REPLACE "/" "\\" DOXYGEN_DOT_PATH "${DOXYGEN_DOT_PATH}")
    message(STATUS "Using dot path: ${DOXYGEN_DOT_PATH}")
  endif()

  doxygen_add_docs(
    documentation
    ALL
    ${documentation_directories}
    COMMENT "Generate HTML documentation"
  )

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
          DESTINATION ./doc)
endif()
