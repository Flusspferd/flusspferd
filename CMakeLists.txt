# vim:ts=4:sw=4:expandtab:autoindent:
#
# The MIT License
#
# Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
#                                      http://flusspferd.org/contributors.txt)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

project(Flusspferd)

cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

if(NOT CMAKE_BUILD_TYPE)
    set(
        CMAKE_BUILD_TYPE
        Debug
        CACHE
        STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

message(STATUS "Source: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary: ${CMAKE_BINARY_DIR}")

if(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files that cmake just created:\nrm -rf CMakeCache.txt CMakeFiles")
endif()

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

set(INSTALL_EXECUTABLES_PATH ${CMAKE_INSTALL_PREFIX}/bin)
set(INSTALL_LIBRARIES_PATH ${CMAKE_INSTALL_PREFIX}/lib)
set(INSTALL_LIBDATA_PATH ${CMAKE_INSTALL_PREFIX}/lib/flusspferd)
set(INSTALL_MODULES_PATH ${CMAKE_INSTALL_PREFIX}/lib/flusspferd/modules)
set(INSTALL_HEADERS_PATH ${CMAKE_INSTALL_PREFIX}/include)
set(INSTALL_ETC_PATH ${CMAKE_INSTALL_PREFIX}/etc)

file(RELATIVE_PATH REL_EXECUTABLES_PATH ${CMAKE_INSTALL_PREFIX} ${INSTALL_EXECUTABLES_PATH})
file(RELATIVE_PATH REL_LIBRARIES_PATH ${CMAKE_INSTALL_PREFIX} ${INSTALL_LIBRARIES_PATH})
file(RELATIVE_PATH REL_LIBDATA_PATH ${CMAKE_INSTALL_PREFIX} ${INSTALL_LIBDATA_PATH})
file(RELATIVE_PATH REL_MODULES_PATH ${CMAKE_INSTALL_PREFIX} ${INSTALL_MODULES_PATH})
file(RELATIVE_PATH REL_ETC_PATH ${CMAKE_INSTALL_PREFIX} ${INSTALL_ETC_PATH})

if(NOT WIN32 AND NOT FLUSSPFERD_VERSION)
    execute_process(
        COMMAND "${Flusspferd_SOURCE_DIR}/flusspferd-version.sh"
        WORKING_DIRECTORY "${Flusspferd_SOURCE_DIR}"
        OUTPUT_VARIABLE FLUSSPFERD_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT FLUSSPFERD_VERSION)
    set(FLUSSPFERD_VERSION unknown)
endif()

add_definitions("-DFLUSSPFERD_VERSION=\"${FLUSSPFERD_VERSION}\"")
add_definitions("-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

message(STATUS "Flusspferd Version " ${FLUSSPFERD_VERSION})

option(RELOCATABLE "Whether the binary should be relocatable" ON)
option(CREATE_DOCUMENTATION "Create Documentation for Flusspferd" OFF)
option(
    FORCE_PLUGINS
    "Fail configuration if some enabled plugins could not be configured"
    OFF)


if(RELOCATABLE)
    add_definitions("-DFLUSSPFERD_RELOCATABLE")
    # TODO: should should probably use some relative_to funciton to be smarter
    # That or being settable from a cmake option
    add_definitions("-DFLUSSPFERD_ETC_PATH=\"../etc/flusspferd\"")
    set(INSTALL_RELATIVE_MODULES_PATH "../lib/flusspferd/modules")
    set(INSTALL_RELATIVE_LIBDATA_PATH "../lib/flusspferd")
else()
    add_definitions("-DFLUSSPFERD_ETC_PATH=\"${INSTALL_ETC_PATH}\"")
endif()

set(CMAKE_SHARED_MODULE_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-DDEBUG")
endif()

#############################################################################

include(CheckIncludeFile)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(FindPkgConfig)

# For the flusspferd_plugin function
include(FlusspferdPlugin)


## Compiler options #########################################################

if(WIN32)
    set(_ENABLE_TESTS_DEFAULT OFF)
else()
    set(_ENABLE_TESTS_DEFAULT ON)
endif()

option(ENABLE_TESTS "Compile the test suite" ${_ENABLE_TESTS_DEFAULT})

if(CMAKE_COMPILER_IS_GNUCXX)
    # MinGW doesn't set this by default
    if(WIN32)
        add_definitions(-DWIN32)
    else()
        # This causes problems with MinGW and #include <string>. swprintf on
        # windows has a non-standard signature
        add_definitions("-ansi")
        # Boost 1.39 on mingw seems to produce a lot of these - they are very
        # noisy
        add_definitions("-Wstrict-aliasing=1")
    endif()

    add_definitions("-pedantic-errors -Wall -Wextra"
        "-Winit-self -Woverloaded-virtual -Wnon-virtual-dtor -Wsign-promo"
        "-Wno-long-long")

    option(ENABLE_DEBUG_WARNINGS "Enable -Wpadded and -Weffc++" OFF)
    if(ENABLE_DEBUG_WARNINGS)
        add_definitions("-Wpadded -Weffc++")
    endif()


    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")

    set(CMAKE_REQUIRED_FLAGS "")

    check_cxx_source_compiles(
        "#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 2)
        #error \"GCC too old\"
        #endif
        int main() {}"
        GCC_VERSION_OK)

    if(NOT GCC_VERSION_OK)
        message(FATAL_ERROR "GCC Version too old! Get a newer GCC.")
    endif()
elseif(MSVC)
    add_definitions("/W3 /wd4512 /wd4355 /wd4800 /wd4996 /wd4714 /EHa")
    set(WINLIBS projects/windows/lib32)
    set(WININCLUDES ${CMAKE_SOURCE_DIR}/${WINLIBS})
endif()

include_directories(${Flusspferd_SOURCE_DIR}/include)

## POSIX ####################################################################

check_cxx_source_runs(
    "#include <unistd.h>
     #include <cstdlib>
     int main()
     {
       return _POSIX_VERSION >= 199506
              ? EXIT_SUCCESS
          : EXIT_FAILURE;
     }"
    FLUSSPFERD_HAVE_POSIX)

if(FLUSSPFERD_HAVE_POSIX)
  check_cxx_source_compiles(
    "#include <sys/epoll.h>
     int main() {
        epoll_create1(0);
     }"
     FLUSSPFERD_HAVE_EPOLL)

   if(FLUSSPFERD_HAVE_EPOLL)
     add_definitions(-DHAVE_EPOLL)
   endif()
endif()

## Boost ####################################################################

set(Boost_USE_MULTITHREADED ON)

if(WIN32)
    set(Boost_USE_STATIC_LIBS ON)
endif()

set(Boost_FIND_REQUIRED TRUE)

set(FLUSSPFERD_BOOST_LIBRARIES thread filesystem system iostreams)

if(ENABLE_TESTS)
    set(FLUSSPFERD_BOOST_LIBRARIES unit_test_framework ${FLUSSPFERD_BOOST_LIBRARIES})
endif()

find_package(Boost 1.40.0 COMPONENTS ${FLUSSPFERD_BOOST_LIBRARIES})

if(Boost_INCLUDE_DIR)
    include_directories(${Boost_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Boost headers not found. You will have to specify them in your build environment or you can use -DBOOST_ROOT or -DBOOST_INCLUDE_DIR to tell cmake where your boost installation is.")
endif()

if(Boost_LIBRARY_DIRS)
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "Boost libraries not found. Please set -DBOOST_ROOT or -DBOOST_LIBRARY_DIR manually.")
endif()

if(NOT Boost_FOUND)
    message(STATUS "Boost not found!")
endif()

set(BOOST_PARAMETER_MAX_ARITY "BOOST_PARAMETER_MAX_ARITY=10")
add_definitions(-D${BOOST_PARAMETER_MAX_ARITY})

## sub-CMakeLists.txt #######################################################

add_subdirectory(libflusspferd)
add_subdirectory(programs)
add_subdirectory(test)
add_subdirectory(plugins/curl)
add_subdirectory(plugins/gmp)
add_subdirectory(plugins/sqlite3)

add_subdirectory(misc/emacs)

## Documentation ############################################################

if(NOT WIN32)
  if(CREATE_DOCUMENTATION)
    message(STATUS "WILL create and install documentation")
    add_custom_target(DOCS ALL
      COMMAND ${Flusspferd_SOURCE_DIR}/util/docs.sh
      WORKING_DIRECTORY ${Flusspferd_SOURCE_DIR})
    install(
      DIRECTORY ${Flusspferd_BINARY_DIR}/html/
      DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/flusspferd/)
  endif()
endif()

