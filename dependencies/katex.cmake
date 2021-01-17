set(katex_home ${CMAKE_CURRENT_LIST_DIR}/katex)
if (EXISTS ${katex_home})
  install(DIRECTORY ${katex_home} DESTINATION dependencies)
  # Add dummy Katex library for less confusion 
  add_library(katex INTERFACE)
endif()
