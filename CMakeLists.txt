cmake_minimum_required(VERSION 3.12)

project(gifscript CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

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


RAGEL_TARGET(gifscript
  ${CMAKE_SOURCE_DIR}/gifscript.rl
  ${CMAKE_CURRENT_BINARY_DIR}/gifscript.cpp
  COMPILE_FLAGS -G2
)

set(BACKEND_SOURCES
  backend/backend.hpp
  backend/c_code.cpp
  backend/c_code.h
)

set(CORE_SOURCES
  version.h
  logger.h
  parser.h
  parser.c
  machine.h
  machine.cpp
  registers.h
  registers.cpp
  ${RAGEL_gifscript_OUTPUTS}
  )

add_executable(gifscript ${BACKEND_SOURCES} ${CORE_SOURCES})

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
  target_compile_options(gifscript PRIVATE -DGIT_VERSION=${GIT_VERSION} -std=c++20 -Wall -Werror -Wno-unused-const-variable)
endif()

target_include_directories(gifscript PRIVATE ${CMAKE_SOURCE_DIR})
# include fmt
target_include_directories(gifscript PRIVATE ${fmt_SOURCE_DIR}/include)
target_link_libraries(gifscript PRIVATE fmt::fmt)

# CMake on windows causes issues with the lemon parser template file
# We need to get the location of the current lemon parser executable
# And (on vcpkg at least) the location of the template file is in the same directory

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
