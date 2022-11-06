include(FetchContent)

FetchContent_Declare(
    Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.1.0
)

FetchContent_MakeAvailable(Catch2)

add_executable(
  unit_tests
  worker_test.cpp
  logger_test.cpp
  queue_test.cpp
  observer_test.cpp
  unordered_map_test.cpp
  pool_test.cpp
)

target_include_directories(unit_tests
   PRIVATE
   ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(unit_tests
  PRIVATE
    Catch2::Catch2WithMain
    pthread
    fmt
)

#Add "-fsanitize=thread -fPIE -pie -g" for data race testing
# target_compile_features(unit_tests PRIVATE cxx_std_20)
set_target_properties(unit_tests
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
   "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   target_compile_options(unit_tests PRIVATE -Wall -Wextra -Werror)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
   target_compile_options(unit_tests PRIVATE /W4 /WX /EHsc)
endif()
