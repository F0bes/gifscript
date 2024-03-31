cmake_minimum_required(VERSION 3.12)

project(gifscript CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

# Using CMAKE_CXX_STANDARD doesn't work for GCC...
# This also doesn't work for Clang 14...
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CPP_23_ARG "-std=c++2b")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CPP_23_ARG "-std=c++23")
endif()

find_package(RAGEL REQUIRED)

if(WIN32)
  find_program(LEMON-PARSER "lemon" REQUIRED)
  if(NOT ${LEMON-PARSER} STREQUAL LEMON-PARSER-NOTFOUND)
    message("lemon parser (lemon.exe) not found. If using VCPKG install lemon-parser-generator and add `$VCPKG_ROOT\\packages\\lemon-parser-generator_x64-windows\\tools\\lemon` (or wherever lemon.exe is) to your $PATH.")
  endif()
endif()

include(FetchContent)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

if(NOT WIN32)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  if(CLANG_TIDY_EXE AND NOT DISABLE_CLANG_TIDY AND NOT WIN32)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,modernize-*,performance-*,portability-*,bugprone-*,clang-analyzer-*)
  endif()
endif()

RAGEL_TARGET(gifscript
  ${CMAKE_SOURCE_DIR}/gifscript.rl
  ${CMAKE_CURRENT_BINARY_DIR}/gifscript.cpp
  COMPILE_FLAGS -G2
)

set(GENERATED_SOURCES
  parser.h
  parser.c
  ${RAGEL_gifscript_OUTPUTS}
)

set(BACKEND_SOURCES
  backend/backend.hpp
  backend/c_code.cpp
  backend/c_code.h
)

set(CORE_SOURCES
  version.h
  logger.h
  machine.h
  machine.cpp
  registers.h
  registers.cpp
  )

add_executable(gifscript ${BACKEND_SOURCES} ${CORE_SOURCES} ${GENERATED_SOURCES})

set_source_files_properties(${GENERATED_SOURCES} PROPERTIES
  SKIP_LINTING ON
  )

if(WIN32)
  target_compile_options(gifscript PRIVATE /std:c++latest)
else()
  execute_process(
    COMMAND git describe --tags --abbrev=4 --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  string(CONCAT GIT_VERSION "\"" ${GIT_VERSION} "\"")
  message("git version: ${GIT_VERSION}")
  target_compile_options(gifscript PRIVATE -DGIT_VERSION=${GIT_VERSION} -Wall -Werror -Wno-unused-const-variable ${CPP_23_ARG})
endif()

target_include_directories(gifscript PRIVATE ${CMAKE_SOURCE_DIR})

target_include_directories(gifscript PRIVATE ${fmt_SOURCE_DIR}/include)
target_link_libraries(gifscript PRIVATE fmt::fmt)

# CMake on windows causes issues with the lemon parser template file
# We need to get the location of the current lemon parser executable
# And (on vcpkg at least) the location of the template file is in the same
# discord as the lemon executable

if(WIN32)
  get_filename_component(LEMON_PARSER_DIR ${LEMON-PARSER} DIRECTORY)
  set(LEMON_PARSER_TEMPLATE "-T${LEMON_PARSER_DIR}/lempar.c")
endif()

add_custom_command(
    OUTPUT parser.c parser.h
    COMMAND lemon -q ${CMAKE_SOURCE_DIR}/parser.y -d${CMAKE_CURRENT_BINARY_DIR} ${LEMON_PARSER_TEMPLATE}
    DEPENDS ${CMAKE_SOURCE_DIR}/parser.y
    USES_TERMINAL
)
