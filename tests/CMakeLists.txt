set(CMAKE_CXX_CLANG_TIDY "")

cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

# Type tests
add_executable(type_tests type_tests.cpp)
target_link_libraries(type_tests gtest_main gcov)
target_include_directories(type_tests PRIVATE ${CORE_INCLUDE})
add_test(NAME type_tests COMMAND type_tests)

# Core tests
add_executable(core_tests core_tests.cpp)
target_link_libraries(core_tests gtest_main gifscript_core gcov)
target_include_directories(core_tests PRIVATE ${CORE_INCLUDE} ${BACKEND_INCLUDE} ${CMAKE_BINARY_DIR})
add_test(NAME core_tests COMMAND core_tests)
