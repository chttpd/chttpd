cmake_minimum_required(VERSION 3.7)
project(carrot 
    VERSION 1.0.0
    LANGUAGES C
)


# verbosity
set(CMAKE_VERBOSE_MAKEFILE OFF)


# project configuration
include(config.cmake)


# public include directory
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/carrot
  ${CMAKE_BINARY_DIR}/carrot
)
configure_file(
  carrot/config.h.in 
  carrot/config.h
  @ONLY
)


# cmake libs
include(cmake/build.cmake)
include(cmake/menu.cmake)
include(cmake/lint.cmake)
include(cmake/valgrind.cmake)
include(cmake/cpack.cmake)
include(cmake/uninstall.cmake)


# core tunneling and networking
add_subdirectory(carrot)


install(TARGETS carrot DESTINATION "lib")
install(FILES 
  ${PROJECT_SOURCE_DIR}/include/carrot/carrot.h 
  DESTINATION "include/carrot"
)


# examples
add_subdirectory(examples)


# test
enable_testing()
add_subdirectory(tests)
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
