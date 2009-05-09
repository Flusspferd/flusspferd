# vim:ts=4:sw=4:expandtab:autoindent:
#
# Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

PROJECT(Flusspferd CXX)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
SET (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

IF(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    MESSAGE(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files that cmake just created:\nrm -rf CMakeCache.txt CMakeFiles")
ENDIF()

SET (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

EXECUTE_PROCESS(
    COMMAND "${Flusspferd_SOURCE_DIR}/flusspferd-version.sh"
    WORKING_DIRECTORY "${Flusspferd_SOURCE_DIR}"
    OUTPUT_VARIABLE FLUSSPFERD_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

ADD_DEFINITIONS("-DFLUSSPFERD_VERSION=\"${FLUSSPFERD_VERSION}\"")
ADD_DEFINITIONS("-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

MESSAGE(STATUS "Flusspferd Version " ${FLUSSPFERD_VERSION})

#############################################################################

INCLUDE (CheckIncludeFile)
INCLUDE (CheckCXXCompilerFlag)
INCLUDE (CheckCXXSourceCompiles)
INCLUDE (CheckCXXSourceRuns)
INCLUDE (FindPkgConfig)

## Compiler options ###

IF (CMAKE_COMPILER_IS_GNUCXX)
    OPTION (ENABLE_DEBUG_WARNINGS "Enable -Wpadded and -Weffc++" OFF)

    ADD_DEFINITIONS ("-pedantic-errors -Wall -Wextra -Wconversion"
    "-Wfloat-equal -Wredundant-decls -Wstrict-null-sentinel -Winit-self"
    "-Woverloaded-virtual -Wnon-virtual-dtor -Wctor-dtor-privacy"
    "-Wsign-promo -Wstrict-aliasing=1")

    IF(ENABLE_DEBUG_WARNINGS)
        ADD_DEFINITIONS("-Wpadded -Weffc++")
    ENDIF()

    ADD_DEFINITIONS(-D BOOST_FILESYSTEM_NO_DEPRECATED)

    SET (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    SET (CMAKE_CXX_FLAGS_DEBUG "-g")

    ADD_DEFINITIONS ("-Wno-long-long -ansi")

    SET (CMAKE_REQUIRED_FLAGS "")
ELSEIF (MSVC)
    ADD_DEFINITIONS ("/W3 /wd4512 /wd4355 /wd4800 /wd4996 /wd4714 /EHa")
    SET (WINLIBS projects/windows/lib32)
    SET (WININCLUDES ${CMAKE_SOURCE_DIR}/${WINLIBS})
ENDIF ()

INCLUDE_DIRECTORIES (${Flusspferd_SOURCE_DIR}/include)

## PkgConfig ################################################################

FIND_PACKAGE(PkgConfig)

## libdl ####################################################################

FIND_LIBRARY(
    DL_LIBRARY
    NAMES dl
    DOC "libdl library")

## Boost ####################################################################

SET(Boost_ADDITIONAL_VERSIONS
    "1.39" "1.39.0" "1.38" "1.38.0" "1.37" "1.37.0" "1.36" "1.36.0")
SET(Boost_USE_MULTITHREADED ON)

FIND_PACKAGE(Boost 1.36.0 COMPONENTS
    unit_test_framework
    thread
    filesystem
    system)

IF(Boost_INCLUDE_DIR)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ELSE()
    MESSAGE(FATAL_ERROR "Boost headers not found. You will have to specify them in your build environment or you can use -DBOOST_ROOT or -DBOOST_INCLUDE_DIR to tell cmake where your boost installation is.")
ENDIF()

IF(Boost_LIBRARY_DIRS)
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ELSE()
    MESSAGE(FATAL_ERROR "Boost libraries not found. Please set -DBOOST_ROOT or -DBOOST_LIBRARY_DIR manually.")
ENDIF()

IF(NOT Boost_FOUND)
    MESSAGE(STATUS "Boost not found!")
ENDIF()

## POSIX ####################################################################

CHECK_CXX_SOURCE_RUNS(
    "#include <unistd.h>
     #include <cstdlib>
     int main()
     {
       return _POSIX_VERSION >= 200100
              ? EXIT_SUCCESS
          : EXIT_FAILURE;
     }"
    FLUSSPFERD_HAVE_POSIX)

## Spidermonkey #############################################################

SET(SPIDERMONKEY_DEFINITIONS -DXP_UNIX -DJS_C_STRINGS_ARE_UTF8)

FIND_LIBRARY(
    SPIDERMONKEY_LIBRARY
    NAMES js mozjs js32
    DOC "Spidermonkey library")

SET(CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
SET(CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})
CHECK_CXX_SOURCE_RUNS(
    "#include <js/jsapi.h>
     #include <stdio.h>
     #include <stdlib.h>
     int main() {
         return ((void*) JS_BeginRequest) ? EXIT_SUCCESS : EXIT_FAILURE;
     }"
    FLUSSPFERD_HAVE_SPIDERMONKEY_THREADSAFE)

IF(FLUSSPFERD_HAVE_SPIDERMONKEY_THREADSAFE)
    SET(SPIDERMONKEY_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS} -DJS_THREADSAFE)
ENDIF()

SET(CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
SET(CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})
CHECK_CXX_SOURCE_RUNS(
    "#include <js/jsapi.h>
     int main() {
     #if JS_VERSION >= 180
       // JS 1.8 allows this to be set at runtime
       return 0;
     # else
       return JS_CStringsAreUTF8() ? 0 : 1;
     #endif
     }"
    FLUSSPFERD_HAVE_SPIDERMONKEY_C_STRINGS_ARE_UTF8)

IF(NOT FLUSSPFERD_HAVE_SPIDERMONKEY_C_STRINGS_ARE_UTF8)
    MESSAGE(FATAL_ERROR "Spidermonkey with UTF-8 support required!")
ENDIF()

ADD_DEFINITIONS(${SPIDERMONKEY_DEFINITIONS})

## libedit ##################################################################

PKG_CHECK_MODULES(LIBEDIT libedit)

IF(LIBEDIT_FOUND)
    ADD_DEFINITIONS(-DHAVE_EDITLINE)
ENDIF()

## libxml2 #################################################################

PKG_CHECK_MODULES(LIBXML2 libxml-2.0>=2.6.0)

IF(LIBXML2_FOUND)
    ADD_DEFINITIONS(-DHAVE_LIBXML2 ${LIBXML2_CFLAGS})
ENDIF()

## Library ##################################################################

FILE(
    GLOB
    flusspferd_library_sources
    ${Flusspferd_SOURCE_DIR}/src/spidermonkey/*.cpp)

ADD_LIBRARY(
    flusspferd
    SHARED
    ${flusspferd_library_sources})

TARGET_LINK_LIBRARIES(
    flusspferd
    ${DL_LIBRARY}
    ${SPIDERMONKEY_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY})

## Programs #################################################################

ADD_EXECUTABLE(
    flusspferd-shell
    src/programs/flusspferd.cpp)

TARGET_LINK_LIBRARIES(
    flusspferd-shell
    flusspferd
    ${LIBEDIT_LIBRARIES})

## IO ######################################################################

FILE(
    GLOB
    flusspferd_io_sources
    ${Flusspferd_SOURCE_DIR}/src/io/*.cpp)

ADD_LIBRARY(
    flusspferd-io
    SHARED
    ${flusspferd_io_sources})

TARGET_LINK_LIBRARIES(
    flusspferd-io
    flusspferd)

## XML ######################################################################

FILE(
    GLOB
    flusspferd_xml_sources
    ${Flusspferd_SOURCE_DIR}/src/xml/*.cpp)

ADD_LIBRARY(
    flusspferd-xml
    SHARED
    ${flusspferd_xml_sources})

TARGET_LINK_LIBRARIES(
    flusspferd-xml
    flusspferd
    ${LIBXML2_LIBRARIES})

## Install ##################################################################

install(
    TARGETS
    flusspferd
    flusspferd-shell
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
