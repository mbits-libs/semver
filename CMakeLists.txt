# Copyright (c) 2020 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

cmake_minimum_required (VERSION 3.12)
project (semver
  VERSION 0.1.0
  DESCRIPTION "Semantic Version type for C++17"
  LANGUAGES CXX)

set(PROJECT_VERSION_STABILITY "-beta") # "", or "-alpha", or "-beta", or "-rc.5"

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  message(STATUS "SemVer: Standalone")
  set(SEMVER_TESTING_DEFAULT ON)
  set(SEMVER_INSTALL_DEFAULT ON)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

else()
  message(STATUS "SemVer: Subdir")
  set(SEMVER_TESTING_DEFAULT OFF)
  set(SEMVER_INSTALL_DEFAULT OFF)
endif()


set(SEMVER_TESTING ${SEMVER_TESTING_DEFAULT} CACHE BOOL "Compile and/or run self-tests")
set(SEMVER_INSTALL ${SEMVER_INSTALL_DEFAULT} CACHE BOOL "Install the library")

if (SEMVER_TESTING)
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  message(STATUS "Python3_EXECUTABLE=${Python3_EXECUTABLE}")

  set(COVERALLS_DIRS include/semver src)
  include(tools/coveralls/Coveralls.cmake)
endif()

set(SEMVER_WALL_FLAGS ON CACHE BOOL "Compile with -Wall/-W4 warning levels")

if (SEMVER_WALL_FLAGS)
  if (MSVC)
    set(ADDITIONAL_WALL_FLAGS
        /permissive-
        /Zc:__cplusplus
        /W4
        /w14242
        /w14254
        /w14263
        /w14265
        /w14287
        /we4289
        /w14296
        /w14311
        /w14545
        /w14546
        /w14547
        /w14549
        /w14555
        /w14619
        /w14640
        /w14826
        /w14905
        /w14906
        /w14928
        /w14946)
  else()
    set(ADDITIONAL_WALL_FLAGS
        -Wall -Wextra
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      list(APPEND ADDITIONAL_WALL_FLAGS -fcolor-diagnostics) # -Wlifetime
    else()
      list(APPEND ADDITIONAL_WALL_FLAGS
        -fdiagnostics-color
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
        )
    endif()
  endif()
endif()

configure_file(src/version.hh.in include/semver/version.hh @ONLY)

add_library(semver STATIC
  src/semver.cc
  src/version.cc
  include/semver/semver
  include/semver/semver.hh
  include/semver/version.hh
  "${CMAKE_CURRENT_BINARY_DIR}/include/semver/version.hh"
  )
target_compile_options(semver PRIVATE ${ADDITIONAL_WALL_FLAGS})
target_compile_features(semver PRIVATE cxx_std_17)
target_include_directories(semver
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(CheckCXXSourceCompiles)
function(check_charconv)
  check_cxx_source_compiles("#include <charconv>
int main() {}" HAS_CHARCONV_)
  set(HAS_CHARCONV ${HAS_CHARCONV_} PARENT)
endfunction()
check_cxx_source_compiles("#include <charconv>
int main() {}" HAS_CHARCONV)

if (NOT HAS_CHARCONV)
  message(FATAL_ERROR "The compiler has no access to <charconv>")
endif()

##################################################################
##  INSTALL
##################################################################

if (SEMVER_INSTALL)
  install(TARGETS semver EXPORT mbits-semver)
  install(EXPORT mbits-semver NAMESPACE "mbits-semver::" DESTINATION lib/cmake)
  install(DIRECTORY include/semver DESTINATION include)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/semver/version.hh" DESTINATION include/semver)
endif()

##################################################################
##  TESTING
##################################################################

if (SEMVER_TESTING)

add_executable(semver-test tests/semver-test.cc)
target_compile_options(semver-test PRIVATE ${ADDITIONAL_WALL_FLAGS})
set_target_properties(semver-test
  PROPERTIES
    FOLDER tests
    )
target_link_libraries(semver-test semver)

add_test(
  NAME semver.exit
  COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/semver-test.py"
    "$<TARGET_FILE:semver-test>"
    )
enable_testing()

endif()
