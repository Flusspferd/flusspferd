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

    option(ENABLE_DEBUG_WARNINGS "Enable -Wpadded and -Weffc++" OFF)

    add_definitions("-pedantic-errors -Wall -Wextra"
        "-Winit-self -Woverloaded-virtual -Wnon-virtual-dtor -Wsign-promo"
        "-Wno-long-long")

    if(ENABLE_DEBUG_WARNINGS)
        add_definitions("-Wpadded -Weffc++")
    endif()

    add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)

    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")


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
option(PLUGIN_CURL "Build cURL plugin" ON)
option(PLUGIN_SQLITE3 "Build SQLite3 plugin" ON)
option(PLUGIN_GMP "Build GMP plugin" ON)
set(
    LINE_EDITOR
    "libedit"
    CACHE
    STRING
    "Select the line editor (none, libedit, readline)"
)
option(
    FORCE_LINE_EDITOR
    "Fail configuration if the specified line editor is not available"
    OFF)
option(CREATE_DOCUMENTATION "Create Documentation for Flusspferd" OFF)

if(NOT LINE_EDITOR STREQUAL "none" AND
   NOT LINE_EDITOR STREQUAL "libedit" AND
   NOT LINE_EDITOR STREQUAL "readline")
 message(FATAL_ERROR "Invalid option given for LINE_EDTIOR: ${LINE_EDITOR}")
endif()

## /proc/self/exe ###########################################################

if(RELOCATABLE)
    add_definitions(-DFLUSSPFERD_RELOCATABLE)
    include(FindSelfExe)
    IF(APPLE)
      SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
          "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -Wl,-x -flat_namespace")
      SET(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS
          "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -Wl,-x -flat_namespace")
    ENDIF(APPLE)

endif()

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
    add_definitions(-DFLUSSPFERD_HAVE_POSIX)
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

set(FLUSSPFERD_BOOST_LIBRARIES thread filesystem system)

if(ENABLE_TESTS)
    set(FLUSSPFERD_BOOST_LIBRARIES
        unit_test_framework ${FLUSSPFERD_BOOST_LIBRARIES})
endif()

find_package(Boost 1.40.0 COMPONENTS
    ${FLUSSPFERD_BOOST_LIBRARIES})

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

set(BOOST_PARAMETER_MAX_ARITY "-DBOOST_PARAMETER_MAX_ARITY=10")
add_definitions(${BOOST_PARAMETER_MAX_ARITY})
set(FLUSSPFERD_EXTERNAL_CFLAGS "${FLUSSPFERD_EXTERNAL_CFLAGS} ${BOOST_PARAMETER_MAX_ARITY}")

## Spidermonkey #############################################################

set(Spidermonkey_REQUIRED TRUE)
find_package(Spidermonkey)

if(NOT SPIDERMONKEY_FOUND)
    message(FATAL_ERROR "Spidermonkey not found")
endif()

if(SPIDERMONKEY_FOUND)
    if (NOT SPIDERMONKEY_UTF8)
        message(FATAL_ERROR "Spidermonkey UTF8 support required but not found")
    endif()

    if (NOT SPIDERMONKEY_JS_CONFIG_HEADER_PATH)
        message(FATAL_ERROR "Spidermonkey with config.h required but not found (Version 1.8.1+ required)")
    endif()
endif()

include_directories(${SPIDERMONKEY_INCLUDE_DIR})
add_definitions(${SPIDERMONKEY_DEFINITIONS})

## libedit ##################################################################

if(LINE_EDITOR STREQUAL "libedit")
  find_package(Libedit)

  if(LIBEDIT_FOUND)
    add_definitions(-DHAVE_EDITLINE)
    include_directories(${LIBEDIT_INCLUDE_DIR})
  endif()

  if(FORCE_LINE_EDITOR AND NOT LIBEDIT_FOUND)
        message(SEND_ERROR "libedit required but it was not found")
  endif()
endif()

## GNU/readline #############################################################

if(LINE_EDITOR STREQUAL "readline")
  message("WARNING: GNU/readline is GPL licensed.")

  find_package(Readline)

  if(READLINE_FOUND)
    add_definitions(-DHAVE_READLINE)
    include_directories(${READLINE_INCLUDE_DIR})
  endif()

  if(FORCE_LINE_EDITOR AND NOT READLINE_FOUND)
        message(SEND_ERROR "readline required but it was not found")
  endif()
endif()

## libcurl ##################################################################

if(PLUGIN_CURL)
    find_package(CURL)

    if(CURL_FOUND)
      set(TMP_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
      set(CMAKE_REQUIRED_LIBRARIES ${CURL_LIBRARIES})

      set(TMP_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
      set(CMAKE_REQUIRED_INCLUDES ${CURL_INCLUDE_DIRS})
      check_cxx_source_runs(
        "#include <curl/curl.h>
         #include <cstdlib>
         int main() {
           return (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19) ?
                  EXIT_SUCCESS :
                  EXIT_FAILURE; 
         }"
         CURL_CORRECT_VERSION)
       set(CMAKE_REQUIRED_LIBRARIES ${TMP_CMAKE_REQUIRED_LIBRARIES})
       set(CMAKE_REQUIRED_INCLUDES ${TMP_CMAKE_REQUIRED_INCLUDES})

       if(NOT CURL_CORRECT_VERSION)
         message(STATUS "CURL version too old (7.19.0 or better required) or error during compilation")
         set(CURL_FOUND FALSE)
       endif()
     endif()

    if(FORCE_PLUGINS AND NOT CURL_FOUND)
        message(SEND_ERROR "CURL plugin required but cURL not found")
    endif()
endif()

## sqlite3 ##################################################################

if(PLUGIN_SQLITE3)
    if (WIN32)
        find_library(
            SQLITE3_LIBRARIES
            NAMES sqlite3
            PATHS "${SQLITE3_LIBRARY_DIRS}")
        IF(SQLITE3_LIBRARIES)
            set(SQLITE3_FOUND ON)
            link_directories(${SQLITE3_LIBRARY_DIRS})
            include_directories(${SQLITE3_INCLUDE_DIRS})
        ENDIF()
    else()
        pkg_check_modules(SQLITE3 sqlite3>=3.4.0)
        include_directories(${SQLITE3_INCLUDE_DIRS})
        link_directories(${SQLITE3_LIBRARY_DIRS})
    endif()

    if(FORCE_PLUGINS AND NOT SQLITE3_FOUND)
        message(SEND_ERROR "SQLite3 plugin required but SQLite3 not found")
    endif()

    if (SQLITE3_STATIC)
        add_library(sqlite3_static STATIC IMPORTED)
        set_property(TARGET sqlite3_static PROPERTY IMPORTED_LOCATION SQLITE3_STATIC)
        set(SQLITE3_LIBRARIES ${SQLITE3_STATIC})
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

set(
    flusspferd_library_sources
    include/flusspferd/aliases.hpp
    include/flusspferd/arguments.hpp
    include/flusspferd/array.hpp
    include/flusspferd/binary.hpp
    include/flusspferd/call_context.hpp
    include/flusspferd/class_description.hpp
    include/flusspferd/class.hpp
    include/flusspferd/context.hpp
    include/flusspferd/convert.hpp
    include/flusspferd/create.hpp
    include/flusspferd/create_on.hpp
    include/flusspferd/current_context_scope.hpp
    include/flusspferd/detail/limit.hpp
    include/flusspferd/encodings.hpp
    include/flusspferd/evaluate.hpp
    include/flusspferd/exception.hpp
    include/flusspferd/function_adapter.hpp
    include/flusspferd/function.hpp
    include/flusspferd/getopt.hpp
    include/flusspferd.hpp
    include/flusspferd/init.hpp
    include/flusspferd/io/binary_stream.hpp
    include/flusspferd/io/file.hpp
    include/flusspferd/io/filesystem-base.hpp
    include/flusspferd/io/io.hpp
    include/flusspferd/io/stream.hpp
    include/flusspferd/load_core.hpp
    include/flusspferd/local_root_scope.hpp
    include/flusspferd/modules.hpp
    include/flusspferd/native_function_base.hpp
    include/flusspferd/native_function.hpp
    include/flusspferd/native_object_base.hpp
    include/flusspferd/object.hpp
    include/flusspferd/properties_functions.hpp
    include/flusspferd/property_attributes.hpp
    include/flusspferd/property_iterator.hpp
    include/flusspferd/root.hpp
    include/flusspferd/security.hpp
    include/flusspferd/spidermonkey/arguments.hpp
    include/flusspferd/spidermonkey/context.hpp
    include/flusspferd/spidermonkey/function.hpp
    include/flusspferd/spidermonkey/init.hpp
    include/flusspferd/spidermonkey/object.hpp
    include/flusspferd/spidermonkey/runtime.hpp
    include/flusspferd/spidermonkey/string.hpp
    include/flusspferd/spidermonkey/value.hpp
    include/flusspferd/string.hpp
    include/flusspferd/string_io.hpp
    include/flusspferd/system.hpp
    include/flusspferd/tracer.hpp
    include/flusspferd/value.hpp
    include/flusspferd/value_io.hpp
    include/flusspferd/version.hpp
    src/core/binary.cpp
    src/core/class.cpp
    src/core/convert.cpp
    src/core/encodings.cpp
    src/core/flusspferd_module.cpp
    src/core/function_adapter.cpp
    src/core/getopt.cpp
    src/core/load_core.cpp
    src/core/modules.cpp
    src/core/properties_functions.cpp
    src/core/property_attributes.cpp
    src/core/security.cpp
    src/core/system.cpp
    src/io/binary_stream.cpp
    src/io/file.cpp
    src/io/filesystem-base.cpp
    src/io/io.cpp
    src/io/stream.cpp
    src/spidermonkey/arguments.cpp
    src/spidermonkey/array.cpp
    src/spidermonkey/context.cpp
    src/spidermonkey/create.cpp
    src/spidermonkey/evaluate.cpp
    src/spidermonkey/exception.cpp
    src/spidermonkey/function.cpp
    src/spidermonkey/init.cpp
    src/spidermonkey/local_root_scope.cpp
    src/spidermonkey/native_function_base.cpp
    src/spidermonkey/native_object_base.cpp
    src/spidermonkey/object.cpp
    src/spidermonkey/property_iterator.cpp
    src/spidermonkey/root.cpp
    src/spidermonkey/string.cpp
    src/spidermonkey/tracer.cpp
    src/spidermonkey/value.cpp
)

if(MSVC)
    # Build static lib for MSVC
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

# Libraries linking against the flusspferd DSO dont need to link to all the above.
set_property(TARGET flusspferd PROPERTY LINK_INTERFACE_LIBRARIES "")


if(WIN32)
    # .dll goes in bin/
    install(TARGETS flusspferd RUNTIME DESTINATION ${INSTALL_EXECUTABLES_PATH})
    # .lib goes in lib/
    install(TARGETS flusspferd ARCHIVE DESTINATION ${INSTALL_LIBRARIES_PATH})
else()
    install(TARGETS flusspferd LIBRARY DESTINATION ${INSTALL_LIBRARIES_PATH})
endif()

## Library - coverage variant ###############################################

if(ENABLE_TESTS)

# The coverage library has been removed for now. It only makes it less likely
# for people to run the tests, which is a bigger problem than getting coverage
# numbers, which don't work for JS anyways.

#    file(
#        GLOB
#        flusspferd_coverage_extra_sources
#        ${Flusspferd_SOURCE_DIR}/src/io/*.cpp
#        ${Flusspferd_SOURCE_DIR}/src/xml/*.cpp)
#
#    add_library(
#        flusspferd-coverage
#        SHARED
#        ${flusspferd_library_sources} ${flusspferd_coverage_extra_sources})
#
#    target_link_libraries(flusspferd-coverage ${flusspferd_LIBS})
#
#    set(COV_FLAGS "-fprofile-arcs -ftest-coverage")
#
#    set_property(
#        TARGET flusspferd-coverage
#        PROPERTY COMPILE_FLAGS
#        "${COV_FLAGS} -DFLUSSPFERD_COVERAGE")
#
#    set_property(
#        TARGET flusspferd-coverage
#        PROPERTY LINK_FLAGS
#        ${COV_FLAGS})

endif()

## Programs #################################################################

add_executable(
    flusspferd-shell
    src/programs/flusspferd.cpp)

set_property(
    TARGET flusspferd-shell
    PROPERTY OUTPUT_NAME
    flusspferd)

set(flusspferd_shell_LIBS flusspferd ${Boost_SYSTEM_LIBRARY})

if(NOT LINE_EDITOR STREQUAL "readline")
  list(APPEND flusspferd_shell_LIBS ${LIBEDIT_LIBRARIES})
else()
  list(APPEND flusspferd_shell_LIBS ${READLINE_LIBRARIES})
endif()

target_link_libraries(
    flusspferd-shell
    ${flusspferd_shell_LIBS})

install(TARGETS
    flusspferd-shell
    RUNTIME DESTINATION ${INSTALL_EXECUTABLES_PATH})

## Tests ###################################################################

if(ENABLE_TESTS)
    set(
      TESTS
      test/test_arguments.cpp
      test/test_array.cpp
      test/test_class_description.cpp
      test/test_context.cpp
      test/test_create.cpp
      test/test_current_context_scope.cpp
      test/test_evaluate.cpp
      test/test_exception.cpp
      test/test_function.cpp
      test/test_init.cpp
      test/test_io.cpp
      test/test_object.cpp
      test/test_property_iterator.cpp
      test/test_regression_159.cpp
      test/test_string.cpp
      test/test_value.cpp
    )

    foreach(TEST_SOURCE ${TESTS})
        string(REGEX MATCH "test_[a-zA-Z0-9_]*" TEST_OUTPUT ${TEST_SOURCE})
        string(REGEX REPLACE "^test_" "" TEST_NAME TEST_OUTPUT)
        set(TEST_SOURCES ${TEST_SOURCE} test/test_environment.hpp)
        add_executable(${TEST_OUTPUT} ${TEST_SOURCES})
        set_property(
            TARGET ${TEST_OUTPUT}
            PROPERTY COMPILE_FLAGS
            "-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MODULE=${TEST_NAME}")
        target_link_libraries(
            ${TEST_OUTPUT}
            ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
            flusspferd)
    endforeach()

endif()

## Plugins #################################################################

if(PLUGIN_CURL AND CURL_FOUND)
    set(PLUGINS ${PLUGINS} curl)
    set(curl_LIBRARIES ${CURL_LIBRARIES})
    set(curl_SOURCES
      src/plugins/curl/curl.cpp
    )
    include_directories(${CURL_INCLUDE_DIR})
endif()

if(PLUGIN_SQLITE3 AND SQLITE3_FOUND)
    set(PLUGINS ${PLUGINS} sqlite3)
    set(sqlite3_DEFINITIONS ${SQLITE3_DEFINITIONS})
    set(sqlite3_LIBRARIES ${SQLITE3_LIBRARIES})
    set(
      sqlite3_SOURCES
      src/plugins/sqlite3/sqlite.cpp
      src/plugins/sqlite3/sqlite_cursor.cpp
      src/plugins/sqlite3/sqlite_cursor.hpp
      src/plugins/sqlite3/sqlite.hpp
      src/plugins/sqlite3/sqlite_plugin.cpp
    )
    set(
      sqlite3_JS_FILES
      src/plugins/sqlite3/sqlite3.js
    )
endif()

if(PLUGIN_GMP AND GMP_FOUND AND GMPXX_FOUND)
    set(PLUGINS ${PLUGINS} gmp)
    set(gmp_LIBRARIES ${GMPXX_LIBRARY} ${GMP_LIBRARY})
    set(
      gmp_SOURCES
      src/plugins/gmp/exception.hpp
      src/plugins/gmp/Float.cpp
      src/plugins/gmp/Float.hpp
      src/plugins/gmp/gmp.cpp
      src/plugins/gmp/Integer.cpp
      src/plugins/gmp/Integer.hpp
      src/plugins/gmp/Rational.cpp
      src/plugins/gmp/Rational.hpp
    )
    set(
      gmp_JS_FILES
      #empty
    )
    include_directories(${GMP_INCLUDE_DIR})
endif()

foreach(PLUGIN ${PLUGINS})
    message(STATUS "Plugin: ${PLUGIN} (will be built)")

    # OSX doesn't work properly with this as MODULE (something about --flat-namespace)
    if (APPLE)
        add_library(${PLUGIN}_PLUGIN SHARED ${${PLUGIN}_SOURCES})
    else()
        add_library(${PLUGIN}_PLUGIN MODULE ${${PLUGIN}_SOURCES})
    endif()

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

    IF (WIN32)
        # Remove the 'lib' prefix from plugin modules on win32
        set_property(
            TARGET ${PLUGIN}_PLUGIN
            PROPERTY PREFIX "")
    ENDIF()

    target_link_libraries(
        ${PLUGIN}_PLUGIN
        flusspferd
        ${${PLUGIN}_LIBRARIES})

    install(TARGETS
        ${PLUGIN}_PLUGIN
        LIBRARY DESTINATION ${INSTALL_MODULES_PATH})

    if(${PLUGIN}_JS_FILES)
        install(FILES
            ${${PLUGIN}_JS_FILES}
            DESTINATION ${INSTALL_MODULES_PATH})
    endif()
endforeach()

set(JS_PLUGINS
    util util/array util/function util/range util/load test test/equiv http/headers)

foreach(JS_PLUGIN ${JS_PLUGINS})

    string(REGEX MATCH ".*/" ${JS_PLUGIN}_DIR ${JS_PLUGIN})

    install(FILES
        src/js/${JS_PLUGIN}.js
        DESTINATION ${INSTALL_MODULES_PATH}/${${JS_PLUGIN}_DIR})

endforeach()

## Headers ##################################################################

file(GLOB_RECURSE HEADERFILES *.hpp)

foreach(HEADER ${HEADERFILES})
  set_source_files_properties(${HEADER}
    PROPERTIES
    HEADER_FILE_ONLY TRUE)
endforeach()

install(
    DIRECTORY include/
    DESTINATION ${INSTALL_HEADERS_PATH}
    FILES_MATCHING PATTERN "*.hpp")

## Prelude ##################################################################

install(
    FILES src/js/prelude.js
    DESTINATION ${INSTALL_LIBDATA_PATH})

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

## Configured files##########################################################

configure_file(
    ${Flusspferd_SOURCE_DIR}/src/js/jsrepl.js.in
    ${Flusspferd_BINARY_DIR}/js/jsrepl.js)

install(
    FILES ${Flusspferd_BINARY_DIR}/js/jsrepl.js
    DESTINATION ${INSTALL_ETC_PATH}/flusspferd)

if(NOT MSVC)
    set(FLUSSPFERD_EXTERNAL_CFLAGS
        "${FLUSSPFERD_EXTERNAL_CFLAGS} -I${Boost_INCLUDE_DIR} -I${SPIDERMONKEY_INCLUDE_DIR}")

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

if (NOT WIN32)
    add_custom_command(
      OUTPUT flusspferd.1
      COMMAND ${Flusspferd_BINARY_DIR}/bin/flusspferd -c /dev/null --hidden-man > ${Flusspferd_BINARY_DIR}/flusspferd.1.options
      COMMAND ${Flusspferd_BINARY_DIR}/bin/flusspferd -c /dev/null -- ${Flusspferd_SOURCE_DIR}/util/replace-var.js --file-var OPTIONS=${Flusspferd_BINARY_DIR}/flusspferd.1.options ${Flusspferd_SOURCE_DIR}/help/flusspferd.1.in > ${Flusspferd_BINARY_DIR}/flusspferd.1
      DEPENDS ${Flusspferd_SOURCE_DIR}/help/flusspferd.1.in
      DEPENDS flusspferd-shell
      COMMENT "Generating manpage")
    add_custom_command(
      OUTPUT flusspferd.1.gz
      COMMAND gzip -9 -c ${Flusspferd_BINARY_DIR}/flusspferd.1 > ${Flusspferd_BINARY_DIR}/flusspferd.1.gz
      DEPENDS flusspferd.1
      COMMENT "Compressing manpage")
    add_custom_target(
      MANPAGE ALL
      DEPENDS flusspferd.1.gz)
    install(
        FILES ${Flusspferd_BINARY_DIR}/flusspferd.1.gz
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)

    add_custom_command(
      OUTPUT bash_completion.sh
      COMMAND ${Flusspferd_BINARY_DIR}/bin/flusspferd -c /dev/null --hidden-bash > ${Flusspferd_BINARY_DIR}/bash_completion.impl
      COMMAND ${Flusspferd_BINARY_DIR}/bin/flusspferd -c /dev/null -- ${Flusspferd_SOURCE_DIR}/util/replace-var.js --file-var IMPL=${Flusspferd_BINARY_DIR}/bash_completion.impl ${Flusspferd_SOURCE_DIR}/misc/bash_completion.sh.in > ${Flusspferd_BINARY_DIR}/bash_completion.sh
      DEPENDS flusspferd-shell
      COMMENT "Generating Bash Completion Script")
    add_custom_target(
      BASH_COMPLETION ALL
      DEPENDS bash_completion.sh)
    install(
      FILES ${Flusspferd_BINARY_DIR}/bash_completion.sh
      DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/bash_completion.d
      RENAME flusspferd)

    find_program(EMACS NAMES emacs)
    if(NOT EMACS STREQUAL "EMACS-NOTFOUND")
      message(STATUS "WILL install Emacs mode")
      add_custom_command(
        OUTPUT flusspferd.elc
        COMMAND emacs -batch -q -f batch-byte-compile ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.el
        COMMAND mv ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.elc ${Flusspferd_BINARY_DIR}/
        DEPENDS ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.el
        COMMENT "Byte compiling Emacs code")
      add_custom_target(
        EMACS_MODE ALL
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
