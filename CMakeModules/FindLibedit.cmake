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

IF(LIBEDIT_INCLUDE_DIR)
  SET(Libedit_FIND_QUIETLY TRUE)
ENDIF()

FIND_PATH(LIBEDIT_INCLUDE_DIR editline/readline.h)

FIND_LIBRARY(LIBEDIT_LIBRARY NAMES edit)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Libedit DEFAULT_MSG LIBEDIT_LIBRARY LIBEDIT_INCLUDE_DIR)

IF(LIBEDIT_FOUND)
  SET( LIBEDIT_LIBRARIES ${LIBEDIT_LIBRARY} )
ELSE(LIBEDIT_FOUND)
  SET( LIBEDIT_LIBRARIES )
ENDIF(LIBEDIT_FOUND)

MARK_AS_ADVANCED( LIBEDIT_LIBRARY LIBEDIT_INCLUDE_DIR )
