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

SET (INSTALL_EXECUTABLES_PATH ${CMAKE_INSTALL_PREFIX}/bin)
SET (INSTALL_LIBRARIES_PATH ${CMAKE_INSTALL_PREFIX}/lib)
SET (INSTALL_MODULES_PATH ${CMAKE_INSTALL_PREFIX}/lib/flusspferd/modules)

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

## Compiler options #########################################################

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

## libdl ####################################################################

SET(DL_REQUIRED TRUE)
FIND_PACKAGE(DL)
INCLUDE_DIRECTORIES(${DL_INCLUDE_DIR})

## Boost ####################################################################

SET(Boost_USE_MULTITHREADED ON)

SET(Boost_FIND_REQUIRED TRUE)

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

SET(Spidermonkey_REQUIRED TRUE)
FIND_PACKAGE(Spidermonkey)

IF(NOT SPIDERMONKEY_UTF8)
    MESSAGE(FATAL_ERROR "Spidermonkey UTF8 support required but not found")
ENDIF()

INCLUDE_DIRECTORIES(${SPIDERMONKEY_INCLUDE_DIR})
ADD_DEFINITIONS(${SPIDERMONKEY_DEFINITIONS})

## libedit ##################################################################

PKG_CHECK_MODULES(LIBEDIT libedit)

IF(LIBEDIT_FOUND)
    ADD_DEFINITIONS(-DHAVE_EDITLINE)
ENDIF()

## libxml2 ##################################################################

PKG_CHECK_MODULES(LIBXML2 libxml-2.0>=2.6.0)

IF(LIBXML2_FOUND)
    ADD_DEFINITIONS(-DHAVE_LIBXML2 ${LIBXML2_CFLAGS})
ENDIF()

## sqlite3 ##################################################################

PKG_CHECK_MODULES(SQLITE3 sqlite3>=3.4.0)

## libcurl ##################################################################

FIND_PACKAGE(CURL)

## GMP ######################################################################

FIND_PACKAGE(GMP REQUIRED)
FIND_PACKAGE(GMPXX REQUIRED)

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
    ${DL_LIBRARIES}
    ${SPIDERMONKEY_LIBRARIES}
    ${Boost_THREAD_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY})

INSTALL(TARGETS
    flusspferd
    LIBRARY DESTINATION ${INSTALL_LIBRARIES_PATH})

## Programs #################################################################

ADD_EXECUTABLE(
    flusspferd-shell
    src/programs/flusspferd.cpp)

SET_PROPERTY(
    TARGET flusspferd-shell
    PROPERTY OUTPUT_NAME
    flusspferd)

TARGET_LINK_LIBRARIES(
    flusspferd-shell
    flusspferd
    ${LIBEDIT_LIBRARIES})

INSTALL(TARGETS
    flusspferd-shell
    RUNTIME DESTINATION ${INSTALL_EXECUTABLES_PATH})

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

INSTALL(TARGETS
    flusspferd-io
    LIBRARY DESTINATION ${INSTALL_LIBRARIES_PATH})

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

INSTALL(TARGETS
    flusspferd-xml
    LIBRARY DESTINATION ${INSTALL_LIBRARIES_PATH})

## Plugins #################################################################

SET(PLUGINS os posix environment SQLite3 cURL gmp)

SET(SQLite3_DEFINITIONS ${SQLITE3_DEFINITIONS})
SET(SQLite3_LIBRARIES ${SQLITE3_LIBRARIES})

SET(cURL_LIBRARIES ${CURL_LIBRARIES})
SET(cURL_INCLUDE_DIRECTORIES ${CURL_INCLUDE_DIR})

SET(gmp_LIBRARIES ${GMPXX_LIBRARY} ${GMP_LIBRARY})
SET(gmp_INCLUDE_DIRECTORIES ${GMP_INCLUDE_DIR})

FOREACH(PLUGIN ${PLUGINS})

    FILE(GLOB
        ${PLUGIN}_sources
        ${Flusspferd_SOURCE_DIR}/src/plugins/${PLUGIN}/*.cpp)

    ADD_LIBRARY(${PLUGIN}_PLUGIN MODULE ${${PLUGIN}_sources})

    SET_PROPERTY(
        TARGET ${PLUGIN}_PLUGIN
        PROPERTY OUTPUT_NAME
        ${PLUGIN})

    SET_PROPERTY(
        TARGET ${PLUGIN}_PLUGIN
        PROPERTY LIBRARY_OUTPUT_DIRECTORY
        ${Flusspferd_BINARY_DIR}/modules)

    SET_PROPERTY(
        TARGET ${PLUGIN}_PLUGIN
        PROPERTY DEFINITIONS
        ${${PLUGIN}_DEFINITIONS})

    SET_PROPERTY(
        TARGET ${PLUGIN}_PLUGIN
        PROPERTY INCLUDE_DIRECTORIES
        ${${PLUGIN}_INCLUDE_DIRECTORIES})

    TARGET_LINK_LIBRARIES(
        ${PLUGIN}_PLUGIN
        flusspferd
        ${${PLUGIN}_LIBRARIES})

    INSTALL(TARGETS
        ${PLUGIN}_PLUGIN
        LIBRARY DESTINATION ${INSTALL_MODULES_PATH})

ENDFOREACH()
