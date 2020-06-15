cmake_minimum_required(VERSION 3.13.1)
project(docs LANGUAGES NONE)

find_package(Doxygen REQUIRED)
find_program(SPHINXBUILD NAMES sphinx-build)
if(NOT SPHINXBUILD)
  message(FATAL_ERROR "The 'sphinx-build' command was not found")
endif()

set(SPINNER_BASE ${CMAKE_SOURCE_DIR}/..)
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/html/doxygen)
set(SPHINXOPTS "" CACHE STRING "Sphinx options")

configure_file(Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile)

add_custom_target(
  doxygen
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${DOXYGEN_OUTPUT_DIRECTORY}
  COMMAND
    ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
)

add_custom_target(
  html
  COMMAND
    ${CMAKE_COMMAND} -E env
    SPINNER_BASE=${SPINNER_BASE}
    DOXYGEN_OUTPUT_DIRECTORY=${DOXYGEN_OUTPUT_DIRECTORY}
    ${SPHINXBUILD} -b html ${SPHINXOPTS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/html
)
