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

if(ICONV_INCLUDE_DIR)
  set(ICONV_FIND_QUIETLY TRUE)
endif()

find_path(ICONV_INCLUDE_DIR iconv.h)

find_library(ICONV_LIBRARY NAMES iconv)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ICONV DEFAULT_MSG ICONV_LIBRARY ICONV_INCLUDE_DIR)

if(ICONV_FOUND)
  set(ICONV_LIBRARIES ${ICONV_LIBRARY})
else(ICONV_FOUND)
  set(ICONV_LIBRARIES)
endif(ICONV_FOUND)

mark_as_advanced(ICONV_LIBRARY ICONV_INCLUDE_DIR)
