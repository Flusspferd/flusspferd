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

include(CheckCXXSourceRuns)
include(CheckIncludeFileCXX)

if(SPIDERMONKEY_INCLUDE_DIR AND SPIDERMONKEY_LIBRARIES)
    set(Spidermonkey_FIND_QUIETLY TRUE)
endif()

if(WIN32)
    set(SPIDERMONKEY_PLATFORM XP_WIN)
else()
    set(SPIDERMONKEY_PLATFORM XP_UNIX)
endif()

set(
    SPIDERMONKEY_DEFINITIONS
    -D${SPIDERMONKEY_PLATFORM} -DJS_C_STRINGS_ARE_UTF8)

find_path(
    SPIDERMONKEY_INCLUDE_DIR
    js/jsapi.h
    PATHS "${SPIDERMONKEY_ROOT}/include"
    NO_DEFAULT_PATH)

find_path(
    SPIDERMONKEY_INCLUDE_DIR
    js/jsapi.h)

find_library(
    SPIDERMONKEY_LIBRARY
    NAMES js mozjs js32
    PATHS "${SPIDERMONKEY_ROOT}/lib"
    NO_DEFAULT_PATH)

find_library(
    SPIDERMONKEY_LIBRARY
    NAMES js mozjs js32)

set(SPIDERMONKEY_LIBRARIES ${SPIDERMONKEY_LIBRARY})

set(CMAKE_REQUIRED_INCLUDES ${SPIDERMONKEY_INCLUDE_DIR})
set(CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
set(CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})
check_cxx_source_runs(
    "#include <js/jsapi.h>
     int main() {
        JSRuntime *rt = JS_NewRuntime(8L * 1024L * 1024L);
        if (!rt)
            return 1;
        return 0;
     }"
    SPIDERMONKEY_RUNS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Spidermonkey
    DEFAULT_MSG
    SPIDERMONKEY_LIBRARIES
    SPIDERMONKEY_INCLUDE_DIR
    SPIDERMONKEY_RUNS)

if(SPIDERMONKEY_FOUND)
    set(CMAKE_REQUIRED_INCLUDES ${SPIDERMONKEY_INCLUDE_DIR})
    set(CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
    set(CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})
    check_include_file_cxx("js/js-config.h" SPIDERMONKEY_JS_CONFIG_HEADER)

    if(NOT SPIDERMONKEY_JS_CONFIG_HEADER)
        set(CMAKE_REQUIRED_INCLUDES ${SPIDERMONKEY_INCLUDE_DIR})
        set(CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
        set(CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})
        check_cxx_source_runs(
            "#include <stdio.h>
             extern \"C\" void js_GetCurrentThread();
             int main() {
                 printf(\"%p\",(void*)js_GetCurrentThread);
                 return(void*)js_GetCurrentThread ? 0 : 1;
             }"
            SPIDERMONKEY_THREADSAFE)

        if(SPIDERMONKEY_THREADSAFE)
            set(
                SPIDERMONKEY_DEFINITIONS
                ${SPIDERMONKEY_DEFINITIONS} -DJS_THREADSAFE)
        endif()
    endif()

    set(CMAKE_REQUIRED_INCLUDES ${SPIDERMONKEY_INCLUDE_DIR})
    set(CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
    set(CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})
    check_cxx_source_runs(
        "#include <js/jsapi.h>
         int main() {
         #if JS_VERSION >= 180
           // JS 1.8 allows this to be set at runtime
           return 0;
         # else
           return JS_CStringsAreUTF8() ? 0 : 1;
         #endif
         }"
        SPIDERMONKEY_UTF8)

endif()

mark_as_advanced(SPIDERMONKEY_INCLUDE_DIR SPIDERMONKEY_LIBRARY)
