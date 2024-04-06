cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(gifscript CXX C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

find_package(RAGEL REQUIRED)

set(CORE_DIR ${CMAKE_SOURCE_DIR}/core)
set(CORE_INCLUDE ${CORE_DIR}/include)
set(CORE_SRC ${CORE_DIR}/src)

set(BACKEND_DIR ${CMAKE_SOURCE_DIR}/backends)
set(BACKEND_INCLUDE ${BACKEND_DIR}/include)
set(BACKEND_SRC ${BACKEND_DIR}/src)

set(FRONTEND_DIR ${CMAKE_SOURCE_DIR}/frontends)

include(FetchContent)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE AND NOT DISABLE_CLANG_TIDY)
  set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,modernize-*,performance-*,portability-*,bugprone-*,clang-analyzer-*)
endif()

RAGEL_TARGET(gifscript
  ${FRONTEND_DIR}/gifscript.rl
  ${CMAKE_CURRENT_BINARY_DIR}/gifscript.cpp
  COMPILE_FLAGS -G2
)

set_source_files_properties(parser.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")

set(GENERATED_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/parser.h
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  ${RAGEL_gifscript_OUTPUTS}
)

set(BACKEND_SOURCES
  ${BACKEND_INCLUDE}/backend.hpp
  ${BACKEND_INCLUDE}/c_code.hpp
  ${BACKEND_INCLUDE}/gifscript_backend.hpp
  ${BACKEND_SRC}/c_code.cpp
  ${BACKEND_SRC}/gifscript_backend.cpp
)

set(CORE_SOURCES
  ${CORE_INCLUDE}/version.hpp
  ${CORE_INCLUDE}/logger.hpp
  ${CORE_INCLUDE}/machine.hpp
  ${CORE_INCLUDE}/registers.hpp
  ${CORE_SRC}/logger.cpp
  ${CORE_SRC}/machine.cpp
  ${CORE_SRC}/registers.cpp
)

add_library(gifscript_core ${BACKEND_SOURCES} ${CORE_SOURCES} ${GENERATED_SOURCES})
target_include_directories(gifscript_core PRIVATE ${CORE_INCLUDE} ${BACKEND_INCLUDE})

add_executable(gifscript gifscript.cpp)
target_include_directories(gifscript PRIVATE ${CORE_INCLUDE} ${BACKEND_INCLUDE})
target_link_libraries(gifscript PRIVATE gifscript_core)

add_executable(tpircsfig ${FRONTEND_DIR}/tpircsfig.cpp)
target_include_directories(tpircsfig PRIVATE ${CORE_INCLUDE} ${BACKEND_INCLUDE} ${CMAKE_BINARY_DIR})
target_link_libraries(tpircsfig PRIVATE gifscript_core)

# DISABLING LINTING FOR WIP TPIRCSFIG FRONTEND FOR NOW
set_source_files_properties(${GENERATED_SOURCES} ${FRONTEND_DIR}/tpircsfig.cpp PROPERTIES
  SKIP_LINTING ON
  )

execute_process(
  COMMAND git describe --tags --abbrev=4 --always
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

string(CONCAT GIT_VERSION "\"" ${GIT_VERSION} "\"")
message("git version: ${GIT_VERSION}")
target_compile_options(gifscript_core PRIVATE -Wall -Werror -Wno-unused-const-variable)
target_compile_options(gifscript PRIVATE -DGIT_VERSION=${GIT_VERSION} -Wall -Werror -Wno-unused-const-variable)
target_compile_options(tpircsfig PRIVATE -DGIT_VERSION=${GIT_VERSION} -Wall -Werror -Wno-unused-const-variable)

target_include_directories(gifscript PUBLIC ${fmt_SOURCE_DIR}/include)
target_link_libraries(gifscript_core PUBLIC fmt::fmt)

add_custom_command(
    OUTPUT parser.cpp parser.h
    COMMAND lemon -q ${CORE_SRC}/parser.y -d${CMAKE_CURRENT_BINARY_DIR} && mv parser.c parser.cpp && sed -i 's/parser.c/parser.cpp/g' parser.cpp
    DEPENDS ${CORE_SRC}/parser.y
    USES_TERMINAL
)

if(NOT DISABLE_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()
