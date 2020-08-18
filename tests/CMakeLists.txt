include(GoogleTest)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        master
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
  unit_tests
  worker_test.cpp
  logger_test.cpp
  queue_test.cpp
  main_test.cpp
)

target_include_directories(unit_tests
   PRIVATE
   ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(unit_tests
  PRIVATE
    gtest
    gmock
    pthread
    fmt
)

#Add "-fsanitize=thread -fPIE -pie -g" for data race testing
target_compile_features(unit_tests PRIVATE cxx_std_17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
   "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   target_compile_options(unit_tests PRIVATE -Wall -Wextra -Werror)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
   target_compile_options(unit_tests PRIVATE /W4 /WX /EHsc)
endif()

gtest_discover_tests(unit_tests)

