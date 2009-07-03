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

project(Flusspferd)

cmake_minimum_required(VERSION 2.6)

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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-DDEBUG")
endif()

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

option(
    ENABLE_TESTS
    "Compile the test suite"
    ${_ENABLE_TESTS_DEFAULT})

if(CMAKE_COMPILER_IS_GNUCXX)
    option(ENABLE_DEBUG_WARNINGS "Enable -Wpadded and -Weffc++" OFF)

    add_definitions("-pedantic-errors -Wall -Wextra"
    "-Winit-self -Woverloaded-virtual -Wnon-virtual-dtor -Wsign-promo"
    "-Wstrict-aliasing=1")

    if(ENABLE_DEBUG_WARNINGS)
        add_definitions("-Wpadded -Weffc++")
    endif()

    add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)

    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")

    add_definitions("-Wno-long-long -ansi")

    set(CMAKE_REQUIRED_FLAGS "")

    check_cxx_source_compiles(
        "#if __GNUC__ < 4
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

## Plugin options ###########################################################

option(
    FORCE_PLUGINS
    "Fail configuration if some enabled plugins could not be configured"
    OFF)
option(PLUGIN_SQLITE3 "Build SQLite3 plugin" ON)
option(PLUGIN_CURL "Build cURL plugin" ON)
option(PLUGIN_GMP "Build GMP plugin" ON)
option(PLUGIN_XML "Build XML plugin" ON)
option(USE_READLINE "Build with GNU/readline instead of libedit" OFF)

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

## libdl ####################################################################

if(FLUSSPFERD_HAVE_POSIX)
    find_package(DL REQUIRED)
    include_directories(${DL_INCLUDE_DIR})
endif()

## iconv ####################################################################

find_package(Iconv REQUIRED)

if(ICONV_ACCEPTS_NONCONST_INPUT)
    add_definitions(-DICONV_ACCEPTS_NONCONST_INPUT)
endif()

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

if(NOT USE_READLINE)
  find_package(Libedit)

  if(LIBEDIT_FOUND)
    add_definitions(-DHAVE_EDITLINE)
    include_directories(${LIBEDIT_INCLUDE_DIR})
  endif()
endif()

## GNU/readline #############################################################

if(USE_READLINE)
  message("WARNING: GNU/readline is GPL licensed.")

  find_package(Readline)

  if(READLINE_FOUND)
    add_definitions(-DHAVE_READLINE)
    include_directories(${READLINE_INCLUDE_DIR})
  endif()
endif()

## libxml2 ##################################################################

if(PLUGIN_XML)
    find_package(LibXml2)

    if(FORCE_PLUGINS AND NOT LIBXML2_FOUND)
        message(SEND_ERROR "XML plugin required but libxml2 not found")
    endif()
endif()

## sqlite3 ##################################################################

if(PLUGIN_SQLITE3)
    pkg_check_modules(SQLITE3 sqlite3>=3.4.0)
    include_directories(${SQLITE3_INCLUDE_DIRS})
    link_directories(${SQLITE3_LIBRARY_DIRS})

    if(FORCE_PLUGINS AND NOT SQLITE3_FOUND)
        message(SEND_ERROR "SQLite3 plugin required but SQLite3 not found")
    endif()
endif()

## libcurl ##################################################################

if(PLUGIN_CURL)
    find_package(CURL)

    if(FORCE_PLUGINS AND NOT CURL_FOUND)
        message(SEND_ERROR "CURL plugin required but cURL not found")
    endif()
endif()

## GMP ######################################################################

if(PLUGIN_GMP)
    find_package(GMP)
    find_package(GMPXX)

    if(FORCE_PLUGINS)
        if(NOT GMP_FOUND OR NOT GMPXX_FOUND)
            message(SEND_ERROR "GMP plugin required but GMP/GMPXX not found")
        endif()
    endif()
endif()

## Library ##################################################################

file(
    GLOB
    flusspferd_library_sources
    ${Flusspferd_SOURCE_DIR}/src/core/*.cpp
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

if(NOT USE_READLINE)
  target_link_libraries(
    flusspferd-shell
    flusspferd
    ${LIBEDIT_LIBRARIES})
else()
  target_link_libraries(
    flusspferd-shell
    flusspferd
    ${READLINE_LIBRARIES})
endif()

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

if(PLUGIN_SQLITE3 AND SQLITE3_FOUND)
    set(PLUGINS ${PLUGINS} sqlite3)
    set(sqlite3_DEFINITIONS ${SQLITE3_DEFINITIONS})
    set(sqlite3_LIBRARIES ${SQLITE3_LIBRARIES})
endif()

if(PLUGIN_CURL AND CURL_FOUND)
    set(PLUGINS ${PLUGINS} curl)
    set(curl_LIBRARIES ${CURL_LIBRARIES})
    include_directories(${CURL_INCLUDE_DIR})
endif()

if(PLUGIN_GMP AND GMP_FOUND AND GMPXX_FOUND)
    set(PLUGINS ${PLUGINS} gmp)
    set(gmp_LIBRARIES ${GMPXX_LIBRARY} ${GMP_LIBRARY})
    include_directories(${GMP_INCLUDE_DIR})
endif()

if(PLUGIN_XML AND LIBXML2_FOUND)
    set(PLUGINS ${PLUGINS} xml)
    set(xml_LIBRARIES ${LIBXML2_LIBRARIES})
    include_directories(${LIBXML2_INCLUDE_DIR})
endif()

foreach(PLUGIN ${PLUGINS})
    message(STATUS "Plugin: ${PLUGIN} (will be built)")

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
        util util/array util/function util/range testharness http/headers)

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

    add_custom_command(
      OUTPUT flusspferd.1.gz
      COMMAND gzip -9 -c ${Flusspferd_SOURCE_DIR}/help/flusspferd.1 > ${Flusspferd_BINARY_DIR}/flusspferd.1.gz
      DEPENDS ${Flusspferd_SOURCE_DIR}/help/flusspferd.1
      COMMENT "Compressing manpage")
    add_custom_target(
      MANPAGE 
      DEPENDS flusspferd.1.gz)

    install(
        FILES ${Flusspferd_BINARY_DIR}/flusspferd.1.gz
        DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)

    find_program(EMACS NAMES emacs)
    if(NOT EMACS-NOTFOUND)
      add_custom_command(
        OUTPUT flusspferd.elc
        COMMAND emacs -batch -q -f batch-byte-compile ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.el
        COMMAND mv ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.elc ${Flusspferd_BINARY_DIR}/
        DEPENDS ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.el
        COMMENT "Byte compiling Emacs code")
      add_custom_target(
        EMACS_MODE
        DEPENDS flusspferd.elc)
      install(
        FILES ${Flusspferd_BINARY_DIR}/flusspferd.elc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/emacs/site-lisp)
    endif()

    # install emacs mode source even if emacs is not found
    install(
      FILES ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.el
      DESTINATION ${CMAKE_INSTALL_PREFIX}/share/emacs/site-lisp)
endif()

message(STATUS "Configuration SUCCESSFUL. No fatal errors.")
