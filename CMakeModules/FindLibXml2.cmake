# vim:ts=4:sw=4:expandtab:autoindent:
# - Try to find LibXml2
# Once done this will define
#
#  LIBXML2_FOUND - System has LibXml2
#  LIBXML2_INCLUDE_DIR - The LibXml2 include directory
#  LIBXML2_LIBRARIES - The libraries needed to use LibXml2
#  LIBXML2_DEFINITIONS - Compiler switches required for using LibXml2
#  LIBXML2_XMLLINT_EXECUTABLE - The XML checking tool xmllint coming with LibXml2

# The MIT License
#
# Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld
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


# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (LIBXML2_INCLUDE_DIR AND LIBXML2_LIBRARIES)
   # in cache already
   SET(LibXml2_FIND_QUIETLY TRUE)
ENDIF (LIBXML2_INCLUDE_DIR AND LIBXML2_LIBRARIES)


IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   FIND_PACKAGE(PkgConfig)
   pkg_check_modules(PC_LIBXML libxml-2.0>=2.6.0)
   SET(LIBXML2_CFLAGS ${PC_LIBXML_CFLAGS_OTHER})
ENDIF (NOT WIN32)

FIND_PATH(LIBXML2_INCLUDE_DIR libxml/xpath.h
   HINTS
   ${PC_LIBXML_INCLUDEDIR}
   ${PC_LIBXML_INCLUDE_DIRS}
   PATH_SUFFIXES libxml2 libxml
   PATHS
   ~/Library/Frameworks
   /Library/Frameworks
   /usr/local
   /usr
   /sw # Fink
   /opt/local # DarwinPorts
   /opt/csw # Blastwave
   /opt
   )


FIND_LIBRARY(LIBXML2_LIBRARIES NAMES xml2 libxml2 libxml
   HINTS
   ${PC_LIBXML_LIBDIR}
   ${PC_LIBXML_LIBRARY_DIRS}
   PATHS
   ~/Library/Frameworks
   /Library/Frameworks
   /usr/local
   /usr
   /sw # Fink
   /opt/local # DarwinPorts
   /opt/csw # Blastwave
   /opt
   )

IF("${LIBXML2_LIBRARIES}" MATCHES "framework")
  SET( LIBXML2_LIBRARIES "${LIBXML2_LIBRARIES}" CACHE STRING "LibXml2 framework")
ENDIF()

IF (LIBXML2_LIBRARIES)
    include(CheckCXXSourceRuns)
    set(CMAKE_REQUIRED_INCLUDES ${LIBXML2_INCLUDE_DIR})
    set(CMAKE_REQUIRED_DEFINITIONS "")
    set(CMAKE_REQUIRED_LIBRARIES ${LIBXML2_LIBRARIES})
    check_cxx_source_runs("
        #include <libxml/xmlversion.h>
        #include <stdio.h>
        int main() { 
          LIBXML_TEST_VERSION;
          if (LIBXML_VERSION < 20600) {
            // This error message doesn't actually get used
            fprintf(stderr, \"Need libxml-2.0 version 2.6.0 minimum, we have %s\\\\n\", LIBXML_DOTTED_VERSION);
            return 1;
          } else {
            return 0;
          }
        }
      "
      LIBXML2_VERSION_GOOD
    )

    IF (NOT LIBXML2_VERSION_GOOD)
      MESSAGE(STATUS "LibXml2 found but version check failed")
      SET(LIBXML2_LIBRARIES "")
      SET(LIBXML2_FOUND 0)
    ENDIF()
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXml2 DEFAULT_MSG LIBXML2_LIBRARIES LIBXML2_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBXML2_INCLUDE_DIR LIBXML2_LIBRARIES)


