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

INCLUDE(CheckCXXSourceRuns)
INCLUDE(CheckIncludeFileCXX)

IF(SPIDERMONKEY_INCLUDE_DIR AND SPIDERMONKEY_LIBRARIES)
    SET(Spidermonkey_FIND_QUIETLY TRUE)
ENDIF()

IF(WIN32)
    SET(SPIDERMONKEY_PLATFORM XP_WIN)
ELSE()
    SET(SPIDERMONKEY_PLATFORM XP_UNIX)
ENDIF()

SET(
    SPIDERMONKEY_DEFINITIONS
    -D${SPIDERMONKEY_PLATFORM} -DJS_C_STRINGS_ARE_UTF8)

FIND_PATH(
    SPIDERMONKEY_INCLUDE_DIR
    js/jsapi.h
    PATHS "${SPIDERMONKEY_ROOT}/include"
    NO_DEFAULT_PATH)

FIND_PATH(
    SPIDERMONKEY_INCLUDE_DIR
    js/jsapi.h)

FIND_LIBRARY(
    SPIDERMONKEY_LIBRARY
    NAMES js mozjs js32
    PATHS "${SPIDERMONKEY_ROOT}/lib"
    NO_DEFAULT_PATH)

FIND_LIBRARY(
    SPIDERMONKEY_LIBRARY
    NAMES js mozjs js32)

SET(SPIDERMONKEY_LIBRARIES ${SPIDERMONKEY_LIBRARY})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    Spidermonkey
    DEFAULT_MSG
    SPIDERMONKEY_LIBRARIES
    SPIDERMONKEY_INCLUDE_DIR)

IF(SPIDERMONKEY_FOUND)
    SET(CMAKE_REQUIRED_INCLUDES ${SPIDERMONKEY_INCLUDE_DIR})
    SET(CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
    SET(CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})
    CHECK_INCLUDE_FILE_CXX("js/js-config.h" SPIDERMONKEY_JS_CONFIG_HEADER)

    IF(NOT SPIDERMONKEY_JS_CONFIG_HEADER)
        SET(CMAKE_REQUIRED_INCLUDES ${SPIDERMONKEY_INCLUDE_DIR})
        SET(CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
        SET(CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})
        CHECK_CXX_SOURCE_RUNS(
            "extern \"C\" void js_GetCurrentThread();
             int main() {
                 return ((void*)js_GetCurrentThread) != 0 ? 0 : 1;
             }"
            SPIDERMONKEY_THREADSAFE)

        IF(SPIDERMONKEY_THREADSAFE)
            SET(
                SPIDERMONKEY_DEFINITIONS
                ${SPIDERMONKEY_DEFINITIONS} -DJS_THREADSAFE)
        ENDIF()
    ENDIF()

    SET(CMAKE_REQUIRED_INCLUDES ${SPIDERMONKEY_INCLUDE_DIR})
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
        SPIDERMONKEY_UTF8)

ENDIF()

MARK_AS_ADVANCED(SPIDERMONKEY_INCLUDE_DIR SPIDERMONKEY_LIBRARY)
