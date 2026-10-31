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
  set(JT_TOOLCHAIN "")
  if(CMAKE_TOOLCHAIN_FILE)   # cross builds (cmake/mingw-w64.cmake): the external project needs the same compiler
    get_filename_component(JT_TC ${CMAKE_TOOLCHAIN_FILE} ABSOLUTE BASE_DIR ${CMAKE_SOURCE_DIR})
    set(JT_TOOLCHAIN -DCMAKE_TOOLCHAIN_FILE=${JT_TC})
  endif()
  ExternalProject_Add(jpegturbo_ext
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/3.1.0.tar.gz
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${JT_PREFIX} -DCMAKE_INSTALL_LIBDIR=lib -DENABLE_SHARED=OFF -DENABLE_STATIC=ON
               -DWITH_SIMD=OFF -DWITH_TURBOJPEG=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON ${JT_TOOLCHAIN}
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

add_executable(megamix src/platform/main.cpp src/platform/devtools.cpp src/platform/devtools_g03.cpp src/platform/devtools_arcade.cpp src/platform/devui_raylib.cpp src/platform/devtools_jobs.cpp src/platform/devtools_menu.cpp src/platform/devtools_save.cpp src/platform/devtools_scnedit.cpp
    src/platform/devtools_g03edit.cpp src/platform/devtools_g03gray.cpp src/platform/devtools_g03pac.cpp src/platform/devtools_g07edit.cpp src/platform/devtools_g01edit.cpp src/platform/devtools_view.cpp src/platform/boot.cpp src/platform/platform_raylib.cpp src/platform/input_raylib.cpp src/platform/gfx_raylib.cpp src/platform/audio_raylib.cpp ${MEGAMIX_SOURCES})
target_include_directories(megamix PRIVATE include)
target_compile_options(megamix PRIVATE -w -fpermissive)
# nlohmann json (header only: the dev window's theme file): the system copy, else downloaded (the portable build always downloads it)
find_path(NLOHMANN_JSON_INCLUDE nlohmann/json.hpp)
if(MEGAMIX_PORTABLE OR NOT NLOHMANN_JSON_INCLUDE)
  include(FetchContent)
  FetchContent_Declare(nlohmann_json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
  FetchContent_MakeAvailable(nlohmann_json)
  set(NLOHMANN_JSON nlohmann_json::nlohmann_json)
else()
  target_include_directories(megamix PRIVATE ${NLOHMANN_JSON_INCLUDE})
endif()
target_link_libraries(megamix PRIVATE raylib ${TURBOJPEG} ${NLOHMANN_JSON})
if(MINGW)   # Windows: no MinGW runtime DLLs next to megamix.exe
  target_link_options(megamix PRIVATE -static -static-libgcc -static-libstdc++)
endif()
