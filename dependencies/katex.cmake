set(katex_home ${CMAKE_CURRENT_LIST_DIR}/katex)
if (EXISTS ${katex_home})
  install(DIRECTORY ${katex_home} DESTINATION dependencies)
endif()
