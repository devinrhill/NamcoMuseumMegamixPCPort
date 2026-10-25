cmake_minimum_required(VERSION 3.20)
project(NamcoMuseumMegamix C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# MEGAMIX_PORTABLE: release build with raylib and libjpeg-turbo built from source and linked statically (no .so to bundle).
# Off by default (it downloads the two sources).
option(MEGAMIX_PORTABLE "release build: static raylib + libjpeg-turbo" OFF)
if(MEGAMIX_PORTABLE)
  include(FetchContent)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(PLATFORM Desktop CACHE STRING "" FORCE)
  FetchContent_Declare(raylib URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz)
  FetchContent_MakeAvailable(raylib)
  # libjpeg-turbo refuses add_subdirectory(): build it as an external project, no SIMD (no nasm on the build host)
  include(ExternalProject)
  set(JT_PREFIX ${CMAKE_BINARY_DIR}/jpegturbo)
  ExternalProject_Add(jpegturbo_ext
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/3.1.0.tar.gz
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${JT_PREFIX} -DCMAKE_INSTALL_LIBDIR=lib -DENABLE_SHARED=OFF -DENABLE_STATIC=ON
               -DWITH_SIMD=OFF -DWITH_TURBOJPEG=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    BUILD_BYPRODUCTS ${JT_PREFIX}/lib/libturbojpeg.a)
  file(MAKE_DIRECTORY ${JT_PREFIX}/include)
  add_library(turbojpeg_static STATIC IMPORTED)
  set_target_properties(turbojpeg_static PROPERTIES IMPORTED_LOCATION ${JT_PREFIX}/lib/libturbojpeg.a
                        INTERFACE_INCLUDE_DIRECTORIES ${JT_PREFIX}/include)
  add_dependencies(turbojpeg_static jpegturbo_ext)
  set(TURBOJPEG turbojpeg_static)
else()
  find_package(raylib 5.0 REQUIRED)
  find_library(TURBOJPEG turbojpeg REQUIRED)   # THP movie frames (libjpeg-turbo)
endif()

# Files that currently compile. Move a file from cmake/pending.txt to
# cmake/sources.cmake once it builds (tools/check_pending.sh does this check).
include(cmake/sources.cmake)

add_executable(megamix src/platform/main.cpp src/platform/devtools.cpp src/platform/boot.cpp src/platform/platform_raylib.cpp src/platform/input_raylib.cpp src/platform/gfx_raylib.cpp src/platform/audio_raylib.cpp ${MEGAMIX_SOURCES})
target_include_directories(megamix PRIVATE include)
target_compile_options(megamix PRIVATE -w -fpermissive)
target_link_libraries(megamix PRIVATE raylib ${TURBOJPEG})
