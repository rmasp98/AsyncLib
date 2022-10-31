cmake_minimum_required(VERSION 3.11)
project(AsyncLib)

# Define if testing should be compiled
set(ASYNCLIB_BUILD_TESTS CACHE BOOL false)

include(FetchContent)
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(fmtlib)

add_library(AsyncLib INTERFACE)

target_include_directories(AsyncLib
   INTERFACE
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(AsyncLib INTERFACE pthread fmt fmtlib)

if ( ASYNCLIB_BUILD_TESTS )
   enable_testing()
   add_subdirectory(tests)
endif()

install(DIRECTORY include/AsyncLib DESTINATION include)
