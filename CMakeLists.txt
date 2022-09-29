cmake_minimum_required (VERSION 3.22 FATAL_ERROR)
project (cc VERSION 0.1 LANGUAGES C CXX)

# Make including this file idempotent
#
if(TARGET cc)
  return()
endif()

# Configuration variables.
#
# Build the tests and set directories if this is the master project
#
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(CC_TEST ON CACHE BOOL "Generate the tests.")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_DOCS_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
  
  # Have cmake output the compilation commands and create a link to it
  # from the project root directory so the language server can find
  # it.
  #
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json)
endif()

# Options for generating tests and documentation.
#
option(CC_TEST "Generate the tests." OFF)
option(CC_DOCS "Generate the docs." OFF)

# Output configuration
#
message("-- cc: Configuration: ${CMAKE_CURRENT_SOURCE_DIR}")
message("-- cc: Included from: ${CMAKE_SOURCE_DIR}")
message("-- cc: CC_TEST ${CC_TEST}")
message("-- cc: CC_DOCS ${CC_DOCS}")
message("-- cc: CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

# Setup compilation before adding dependencies
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)
add_compile_options(-stdlib=libc++)
add_compile_options(-O)
add_link_options(-stdlib=libc++)

# Add our dependencies
#
include(cmake/cpp_core_find_package.cmake)
cpp_core_find_package(util)

# Build the library
#
set(SOURCES
  cc/barrier
  cc/executor
  cc/gate
  cc/latch
  cc/pipe
  cc/ring/cursor
  cc/ring/dependency_graph)

foreach(NAME ${SOURCES})
  list(APPEND FILES "src/core/${NAME}.cpp")
endforeach()

add_library(cc ${FILES})
add_library(cc::cc ALIAS cc)

# Define the header files for this library.
#
file(GLOB_RECURSE PUBLIC_INCLUDE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*.h)
target_sources(cc PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${PUBLIC_INCLUDE_FILES})

target_link_libraries(cc PUBLIC util::util)

# Optionally configure the tests
#
if(CC_TEST)
  enable_testing()
  if (NOT TARGET check)
    add_custom_target(check ${CMAKE_CTEST_COMMAND})
    add_custom_target(check_detail ${CMAKE_CTEST_COMMAND} -V)
  endif()
  add_subdirectory(test)
endif()

# Optionally configure the documentation
#
# if(CC_DOCS)
#   add_subdirectory(docs)
# endif()

# Installation
#
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/install_config.cmake)
install_config(cc)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/core/cc DESTINATION include/core)
