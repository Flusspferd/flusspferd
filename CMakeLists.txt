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

project(Flusspferd CXX)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

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

execute_process(
    COMMAND "${Flusspferd_SOURCE_DIR}/flusspferd-version.sh"
    WORKING_DIRECTORY "${Flusspferd_SOURCE_DIR}"
    OUTPUT_VARIABLE FLUSSPFERD_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT FLUSSPFERD_VERSION)
    set(FLUSSPFERD_VERSION unknown)
endif()

add_definitions("-DFLUSSPFERD_VERSION=\"${FLUSSPFERD_VERSION}\"")
add_definitions("-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

message(STATUS "Flusspferd Version " ${FLUSSPFERD_VERSION})

set(CMAKE_SHARED_MODULE_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
add_definitions("-DFLUSSPFERD_MODULE_SUFFIX=\"${CMAKE_SHARED_MODULE_SUFFIX}\"")

#############################################################################

include(CheckIncludeFile)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(FindPkgConfig)

## Compiler options #########################################################

if(WIN32)
    set(_ENABLE_TESTS_DEFAULT OFF)
else()
    set(_ENABLE_TESTS_DEFAULT ON)
endif()

set(ENABLE_TESTS ${_ENABLE_TESTS_DEFAULT} CACHE BOOL "Compile the test suite")

if(CMAKE_COMPILER_IS_GNUCXX)
    option(ENABLE_DEBUG_WARNINGS "Enable -Wpadded and -Weffc++" OFF)

    add_definitions("-pedantic-errors -Wall -Wextra -Wstrict-null-sentinel"
    "-Winit-self -Woverloaded-virtual -Wnon-virtual-dtor -Wsign-promo"
    "-Wstrict-aliasing=1")

    if(ENABLE_DEBUG_WARNINGS)
        add_definitions("-Wpadded -Weffc++")
    endif()

    add_definitions(-D BOOST_FILESYSTEM_NO_DEPRECATED)

    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")

    add_definitions("-Wno-long-long -ansi")

    set(CMAKE_REQUIRED_FLAGS "")
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
       return _POSIX_VERSION >= 200100
              ? EXIT_SUCCESS
          : EXIT_FAILURE;
     }"
    FLUSSPFERD_HAVE_POSIX)

## libdl ####################################################################

if(FLUSSPFERD_HAVE_POSIX)
    find_package(DL REQUIRED)
    include_directories(${DL_INCLUDE_DIR})
endif()

## iconv ####################################################################

find_package(Iconv REQUIRED)

## Boost ####################################################################

set(Boost_USE_MULTITHREADED ON)

if(WIN32)
    set(Boost_USE_STATIC_LIBS ON)
endif()

set(Boost_FIND_REQUIRED TRUE)

find_package(Boost 1.36.0 COMPONENTS
    unit_test_framework
    thread
    filesystem
    system)

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

## Spidermonkey #############################################################

set(Spidermonkey_REQUIRED TRUE)
find_package(Spidermonkey)

if(NOT SPIDERMONKEY_UTF8)
    message(FATAL_ERROR "Spidermonkey UTF8 support required but not found")
endif()

include_directories(${SPIDERMONKEY_INCLUDE_DIR})
add_definitions(${SPIDERMONKEY_DEFINITIONS})

## libedit ##################################################################

find_package(Libedit)

if(LIBEDIT_FOUND)
    add_definitions(-DHAVE_EDITLINE)
    include_directories(${LIBEDIT_INCLUDE_DIR})
endif()

## libxml2 ##################################################################

pkg_check_modules(LIBXML2 libxml-2.0>=2.6.0)

if(LIBXML2_FOUND)
    add_definitions(-DHAVE_LIBXML2 ${LIBXML2_CFLAGS})
    include_directories(${LIBXML2_INCLUDE_DIRS})
    link_directories(${LIBXML2_LIBRARY_DIRS})
endif()

## sqlite3 ##################################################################

pkg_check_modules(SQLITE3 sqlite3>=3.4.0)
include_directories(${SQLITE3_INCLUDE_DIRS})
link_directories(${SQLITE3_LIBRARY_DIRS})

## libcurl ##################################################################

find_package(CURL)

## GMP ######################################################################

find_package(GMP)
find_package(GMPXX)

## Library ##################################################################

file(
    GLOB
    flusspferd_library_sources
    ${Flusspferd_SOURCE_DIR}/src/spidermonkey/*.cpp
    ${Flusspferd_SOURCE_DIR}/src/io/*.cpp)

if(WIN32)
    set(FLUSSPFERD_LIBRARY_TYPE STATIC)
else()
    set(FLUSSPFERD_LIBRARY_TYPE SHARED)
endif()

add_library(
    flusspferd
    ${FLUSSPFERD_LIBRARY_TYPE}
    ${flusspferd_library_sources})

set(
    flusspferd_LIBS
    ${DL_LIBRARIES}
    ${ICONV_LIBRARIES}
    ${SPIDERMONKEY_LIBRARIES}
    ${Boost_THREAD_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY})

target_link_libraries(flusspferd ${flusspferd_LIBS})

if(NOT WIN32)
    install(TARGETS
        flusspferd
        LIBRARY DESTINATION ${INSTALL_LIBRARIES_PATH})
endif()

## Library - coverage variant ###############################################

if(ENABLE_TESTS)

#    file(
#        GLOB
#        flusspferd_coverage_extra_sources
#        ${Flusspferd_SOURCE_DIR}/src/io/*.cpp
#        ${Flusspferd_SOURCE_DIR}/src/xml/*.cpp)

    add_library(
        flusspferd-coverage
        SHARED
        ${flusspferd_library_sources} ${flusspferd_coverage_extra_sources})

    target_link_libraries(flusspferd-coverage ${flusspferd_LIBS})

    set(COV_FLAGS "-fprofile-arcs -ftest-coverage")

    set_property(
        TARGET flusspferd-coverage
        PROPERTY COMPILE_FLAGS
        "${COV_FLAGS} -DFLUSSPFERD_COVERAGE")

    set_property(
        TARGET flusspferd-coverage
        PROPERTY LINK_FLAGS
        ${COV_FLAGS})

endif()

## Programs #################################################################

add_executable(
    flusspferd-shell
    src/programs/flusspferd.cpp)

set_property(
    TARGET flusspferd-shell
    PROPERTY OUTPUT_NAME
    flusspferd)

target_link_libraries(
    flusspferd-shell
    flusspferd
    ${LIBEDIT_LIBRARIES})

if(NOT WIN32)
    install(TARGETS
        flusspferd-shell
        RUNTIME DESTINATION ${INSTALL_EXECUTABLES_PATH})
endif()

## Tests ###################################################################

if(ENABLE_TESTS)

    file(GLOB
        TESTS
        test/test_*.cpp)

    foreach(TEST_SOURCE ${TESTS})
        string(REGEX MATCH "test_[a-zA-Z0-9_]*" TEST_OUTPUT ${TEST_SOURCE})
        string(REGEX REPLACE "^test_" "" TEST_NAME TEST_OUTPUT)
        add_executable(${TEST_OUTPUT} ${TEST_SOURCE})
        set_property(
            TARGET ${TEST_OUTPUT}
            PROPERTY COMPILE_FLAGS
            "-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MODULE=${TEST_NAME}")
        target_link_libraries(
            ${TEST_OUTPUT}
            ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
            flusspferd-coverage)
    endforeach()

endif()

## Plugins #################################################################

set(PLUGINS os environment)

if(FLUSSPFERD_HAVE_POSIX)
    set(PLUGINS ${PLUGINS} posix)
endif()

if(SQLITE3_FOUND)
    set(PLUGINS ${PLUGINS} SQLite3)
    set(SQLite3_DEFINITIONS ${SQLITE3_DEFINITIONS})
    set(SQLite3_LIBRARIES ${SQLITE3_LIBRARIES})
endif()

if(CURL_FOUND)
    set(PLUGINS ${PLUGINS} cURL)
    set(cURL_LIBRARIES ${CURL_LIBRARIES})
    set(cURL_INCLUDE_DIRECTORIES ${CURL_INCLUDE_DIR})
endif()

if(GMP_FOUND AND GMPXX_FOUND)
    set(PLUGINS ${PLUGINS} gmp)
    set(gmp_LIBRARIES ${GMPXX_LIBRARY} ${GMP_LIBRARY})
    set(gmp_INCLUDE_DIRECTORIES ${GMP_INCLUDE_DIR})
endif()

if(LIBXML2_FOUND)
    set(PLUGINS ${PLUGINS} XML)
    set(XML_LIBRARIES ${LIBXML2_LIBRARIES})
endif()

foreach(PLUGIN ${PLUGINS})

    file(GLOB
        ${PLUGIN}_sources
        ${Flusspferd_SOURCE_DIR}/src/plugins/${PLUGIN}/*.cpp)

    add_library(${PLUGIN}_PLUGIN MODULE ${${PLUGIN}_sources})

    set_property(
        TARGET ${PLUGIN}_PLUGIN
        PROPERTY OUTPUT_NAME
        ${PLUGIN})

    set_property(
        TARGET ${PLUGIN}_PLUGIN
        PROPERTY LIBRARY_OUTPUT_DIRECTORY
        ${Flusspferd_BINARY_DIR}/modules)

    set_property(
        TARGET ${PLUGIN}_PLUGIN
        PROPERTY DEFINITIONS
        ${${PLUGIN}_DEFINITIONS})

    set_property(
        TARGET ${PLUGIN}_PLUGIN
        PROPERTY INCLUDE_DIRECTORIES
        ${${PLUGIN}_INCLUDE_DIRECTORIES})

    target_link_libraries(
        ${PLUGIN}_PLUGIN
        flusspferd
        ${${PLUGIN}_LIBRARIES})

    if(NOT WIN32)
        install(TARGETS
            ${PLUGIN}_PLUGIN
            LIBRARY DESTINATION ${INSTALL_MODULES_PATH})

        file(GLOB
            ${PLUGIN}_JS_FILES
            ${Flusspferd_SOURCE_DIR}/src/plugins/${PLUGIN}/${PLUGIN}.js)

        if(${PLUGIN}_JS_FILES)
            install(FILES
                ${${PLUGIN}_JS_FILES}
                DESTINATION ${INSTALL_MODULES_PATH})
        endif()
    endif()

endforeach()

if(NOT WIN32)
    set(JS_PLUGINS
        Util Util/Array Util/Function Util/Range TestHarness HTTP/Headers)

    foreach(JS_PLUGIN ${JS_PLUGINS})

        string(REGEX MATCH ".*/" ${JS_PLUGIN}_DIR ${JS_PLUGIN})

        install(FILES
            src/js/${JS_PLUGIN}.js
            DESTINATION ${INSTALL_MODULES_PATH}/${${JS_PLUGIN}_DIR})

    endforeach()
endif()

## Headers ##################################################################

if(NOT WIN32)
    install(
        DIRECTORY include/
        DESTINATION ${INSTALL_HEADERS_PATH}
        FILES_MATCHING PATTERN "*.hpp")
endif()

## Prelude ##################################################################

if(NOT WIN32)
    install(
        FILES src/js/prelude.js
        DESTINATION ${INSTALL_LIBDATA_PATH})
endif()

## Configured files##########################################################

if(NOT WIN32)
    configure_file(
        ${Flusspferd_SOURCE_DIR}/src/js/jsrepl.js.in
        ${Flusspferd_BINARY_DIR}/js/jsrepl.js)

    install(
        FILES ${Flusspferd_BINARY_DIR}/js/jsrepl.js
        DESTINATION ${INSTALL_ETC_PATH}/flusspferd)

    set(FLUSSPFERD_EXTERNAL_CFLAGS
        "-I${Boost_INCLUDE_DIR} -I${SPIDERMONKEY_INCLUDE_DIR}")

    foreach(_FLAG @SPIDERMONKEY_DEFINITIONS@)
        set(FLUSSPFERD_EXTERNAL_CFLAGS "${FLUSSPFERD_EXTERNAL_CFLAGS} ${_FLAG}")
    endforeach()

    configure_file(
        ${Flusspferd_SOURCE_DIR}/flusspferd.pc.in
        ${Flusspferd_BINARY_DIR}/flusspferd.pc
        @ONLY)

    install(
        FILES ${Flusspferd_BINARY_DIR}/flusspferd.pc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
endif()
