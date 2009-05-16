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
SET (INSTALL_LIBDATA_PATH ${CMAKE_INSTALL_PREFIX}/lib/flusspferd)
SET (INSTALL_MODULES_PATH ${CMAKE_INSTALL_PREFIX}/lib/flusspferd/modules)
SET (INSTALL_HEADERS_PATH ${CMAKE_INSTALL_PREFIX}/include)
SET (INSTALL_ETC_PATH ${CMAKE_INSTALL_PREFIX}/etc)

EXECUTE_PROCESS(
    COMMAND "${Flusspferd_SOURCE_DIR}/flusspferd-version.sh"
    WORKING_DIRECTORY "${Flusspferd_SOURCE_DIR}"
    OUTPUT_VARIABLE FLUSSPFERD_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

ADD_DEFINITIONS("-DFLUSSPFERD_VERSION=\"${FLUSSPFERD_VERSION}\"")
ADD_DEFINITIONS("-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

MESSAGE(STATUS "Flusspferd Version " ${FLUSSPFERD_VERSION})

SET(CMAKE_SHARED_MODULE_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
ADD_DEFINITIONS("-DFLUSSPFERD_MODULE_SUFFIX=\"${CMAKE_SHARED_MODULE_SUFFIX}\"")

#############################################################################

INCLUDE (CheckIncludeFile)
INCLUDE (CheckCXXCompilerFlag)
INCLUDE (CheckCXXSourceCompiles)
INCLUDE (CheckCXXSourceRuns)
INCLUDE (FindPkgConfig)

## Compiler options #########################################################

SET (ENABLE_TESTS ON CACHE BOOL "Compile the test suite")

IF (CMAKE_COMPILER_IS_GNUCXX)
    OPTION (ENABLE_DEBUG_WARNINGS "Enable -Wpadded and -Weffc++" OFF)

    ADD_DEFINITIONS ("-pedantic-errors -Wall -Wextra"
    "-Wredundant-decls -Wstrict-null-sentinel -Winit-self"
    "-Woverloaded-virtual -Wnon-virtual-dtor"
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

## libdl ####################################################################

IF(FLUSSPFERD_HAVE_POSIX)
    FIND_PACKAGE(DL REQUIRED)
    INCLUDE_DIRECTORIES(${DL_INCLUDE_DIR})
ENDIF()

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

## Spidermonkey #############################################################

SET(Spidermonkey_REQUIRED TRUE)
FIND_PACKAGE(Spidermonkey)

IF(NOT SPIDERMONKEY_UTF8)
    MESSAGE(FATAL_ERROR "Spidermonkey UTF8 support required but not found")
ENDIF()

INCLUDE_DIRECTORIES(${SPIDERMONKEY_INCLUDE_DIR})
ADD_DEFINITIONS(${SPIDERMONKEY_DEFINITIONS})

## libedit ##################################################################

FIND_PACKAGE(Libedit)

IF(LIBEDIT_FOUND)
    ADD_DEFINITIONS(-DHAVE_EDITLINE)
    INCLUDE_DIRECTORIES(${LIBEDIT_INCLUDE_DIR})
ENDIF()

## libxml2 ##################################################################

PKG_CHECK_MODULES(LIBXML2 libxml-2.0>=2.6.0)

IF(LIBXML2_FOUND)
    ADD_DEFINITIONS(-DHAVE_LIBXML2 ${LIBXML2_CFLAGS})
    INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIRS})
    LINK_DIRECTORIES(${LIBXML2_LIBRARY_DIRS})
ENDIF()

## sqlite3 ##################################################################

PKG_CHECK_MODULES(SQLITE3 sqlite3>=3.4.0)
INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIRS})
LINK_DIRECTORIES(${SQLITE3_LIBRARY_DIRS})

## libcurl ##################################################################

FIND_PACKAGE(CURL)

## GMP ######################################################################

FIND_PACKAGE(GMP)
FIND_PACKAGE(GMPXX)

## Library ##################################################################

FILE(
    GLOB
    flusspferd_library_sources
    ${Flusspferd_SOURCE_DIR}/src/spidermonkey/*.cpp)

ADD_LIBRARY(
    flusspferd
    SHARED
    ${flusspferd_library_sources})

SET(
    flusspferd_LIBS
    ${DL_LIBRARIES}
    ${SPIDERMONKEY_LIBRARIES}
    ${Boost_THREAD_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY})

TARGET_LINK_LIBRARIES(flusspferd ${flusspferd_LIBS})

INSTALL(TARGETS
    flusspferd
    LIBRARY DESTINATION ${INSTALL_LIBRARIES_PATH})

## Library - coverage variant ###############################################

IF(ENABLE_TESTS)

#    FILE(
#        GLOB
#        flusspferd_coverage_extra_sources
#        ${Flusspferd_SOURCE_DIR}/src/io/*.cpp
#        ${Flusspferd_SOURCE_DIR}/src/xml/*.cpp)

    ADD_LIBRARY(
        flusspferd-coverage
        SHARED
        ${flusspferd_library_sources} ${flusspferd_coverage_extra_sources})

    TARGET_LINK_LIBRARIES(flusspferd-coverage ${flusspferd_LIBS})

    SET(COV_FLAGS "-fprofile-arcs -ftest-coverage")

    SET_PROPERTY(
        TARGET flusspferd-coverage
        PROPERTY COMPILE_FLAGS
        "${COV_FLAGS} -DFLUSSPFERD_COVERAGE")

    SET_PROPERTY(
        TARGET flusspferd-coverage
        PROPERTY LINK_FLAGS
        ${COV_FLAGS})

ENDIF()

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

## Tests ###################################################################

IF(ENABLE_TESTS)

    FILE(GLOB
        TESTS
        test/test_*.cpp)

    FOREACH(TEST_SOURCE ${TESTS})
        STRING(REGEX MATCH "test_[a-zA-Z0-9_]*" TEST_OUTPUT ${TEST_SOURCE})
        STRING(REGEX REPLACE "^test_" "" TEST_NAME TEST_OUTPUT)
        ADD_EXECUTABLE(${TEST_OUTPUT} ${TEST_SOURCE})
        SET_PROPERTY(
            TARGET ${TEST_OUTPUT}
            PROPERTY COMPILE_FLAGS
            "-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MODULE=${TEST_NAME}")
        TARGET_LINK_LIBRARIES(
            ${TEST_OUTPUT}
            ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
            flusspferd-coverage)
    ENDFOREACH()

ENDIF()

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

INSTALL(TARGETS
    flusspferd-io
    LIBRARY DESTINATION ${INSTALL_MODULES_PATH})

## XML ######################################################################

IF(LIBXML2_FOUND)

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

    INSTALL(TARGETS
        flusspferd-xml
        LIBRARY DESTINATION ${INSTALL_MODULES_PATH})

ENDIF()

## Plugins #################################################################

SET(PLUGINS os environment)

IF(FLUSSPFERD_HAVE_POSIX)
    SET(PLUGINS ${PLUGINS} posix)
ENDIF()

IF(SQLITE3_FOUND)
    SET(PLUGINS ${PLUGINS} SQLite3)
    SET(SQLite3_DEFINITIONS ${SQLITE3_DEFINITIONS})
    SET(SQLite3_LIBRARIES ${SQLITE3_LIBRARIES})
ENDIF()

IF(CURL_FOUND)
    SET(PLUGINS ${PLUGINS} cURL)
    SET(cURL_LIBRARIES ${CURL_LIBRARIES})
    SET(cURL_INCLUDE_DIRECTORIES ${CURL_INCLUDE_DIR})
ENDIF()

IF(GMP_FOUND AND GMPXX_FOUND)
    SET(PLUGINS ${PLUGINS} gmp)
    SET(gmp_LIBRARIES ${GMPXX_LIBRARY} ${GMP_LIBRARY})
    SET(gmp_INCLUDE_DIRECTORIES ${GMP_INCLUDE_DIR})
ENDIF()

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

    FILE(GLOB
        ${PLUGIN}_JS_FILES
        ${Flusspferd_SOURCE_DIR}/src/plugins/${PLUGIN}/${PLUGIN}.js)

    IF(${PLUGIN}_JS_FILES)
        INSTALL(FILES
            ${${PLUGIN}_JS_FILES}
            DESTINATION ${INSTALL_MODULES_PATH})
    ENDIF()

ENDFOREACH()

SET(JS_PLUGINS
    Util Util/Array Util/Function Util/Range TestHarness HTTP/Headers)

FOREACH(JS_PLUGIN ${JS_PLUGINS})

    STRING(REGEX MATCH ".*/" ${JS_PLUGIN}_DIR ${JS_PLUGIN})

    INSTALL(FILES
        src/js/${JS_PLUGIN}.js
        DESTINATION ${INSTALL_MODULES_PATH}/${${JS_PLUGIN}_DIR})

ENDFOREACH()

## Headers ##################################################################

INSTALL(
    DIRECTORY include/
    DESTINATION ${INSTALL_HEADERS_PATH}
    FILES_MATCHING PATTERN "*.hpp")

## Prelude ##################################################################

INSTALL(
    FILES src/js/prelude.js
    DESTINATION ${INSTALL_LIBDATA_PATH})

## Configured files##########################################################

CONFIGURE_FILE(
    ${Flusspferd_SOURCE_DIR}/src/js/jsrepl.js.in
    ${Flusspferd_BINARY_DIR}/js/jsrepl.js)

INSTALL(
    FILES ${Flusspferd_BINARY_DIR}/js/jsrepl.js
    DESTINATION ${INSTALL_ETC_PATH}/flusspferd)

CONFIGURE_FILE(
    ${Flusspferd_SOURCE_DIR}/flusspferd.pc.in
    ${Flusspferd_BINARY_DIR}/flusspferd.pc
    @ONLY)

INSTALL(
    FILES ${Flusspferd_BINARY_DIR}/flusspferd.pc
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
