cmake_minimum_required(VERSION 3.11)
project(AsyncLib)

# Define if testing should be compiled
set(ASYNCLIB_BUILD_TESTS CACHE BOOL false)

add_library(AsyncLib INTERFACE)

target_include_directories(AsyncLib
   INTERFACE
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)


if ( ASYNCLIB_BUILD_TESTS )
   enable_testing()
   add_subdirectory(tests)
endif()

install(DIRECTORY include/AsyncLib DESTINATION include)
