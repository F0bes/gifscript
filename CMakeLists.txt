cmake_minimum_required(VERSION 3.12)

project(gifscript CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

find_package(RAGEL REQUIRED)

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
  logger.h
  parser.h
  parser.c
  machine.h
  machine.cpp
  ${RAGEL_gifscript_OUTPUTS}
  )

add_executable(gifscript ${BACKEND_SOURCES} ${CORE_SOURCES})

target_compile_options(gifscript PRIVATE -std=c++20)
target_include_directories(gifscript PRIVATE ${CMAKE_SOURCE_DIR})
# include fmt
target_include_directories(gifscript PRIVATE ${fmt_SOURCE_DIR}/include)
target_link_libraries(gifscript PRIVATE fmt::fmt)
message(error ${fmt_SOURCE_DIR})

add_custom_command(
    OUTPUT parser.c parser.h
    COMMAND lemon -q ${CMAKE_SOURCE_DIR}/parser.y -d${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_SOURCE_DIR}/parser.y
)
