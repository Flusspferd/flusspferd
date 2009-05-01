// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Ash Berlin, Rüdiger Sonderfeld

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/security.hpp"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <cerrno>

using namespace flusspferd;

namespace {

int errno_getter() {
  return errno;
}

void errno_setter(int errno_) {
  errno = errno_;
}

std::string strerror_(int errno_) {
  return std::strerror(errno_);
}

// import hook
extern "C" void flusspferd_load(object posix)
{
  local_root_scope scope;

#ifdef HAVE_FORK
  // TODO: Need to work out how to make this play nice with the interactive repl
  create_native_function(posix, "fork", &::fork);
#endif

  create_native_function(posix, "sleep", &::sleep);

#ifdef HAVE_USLEEP
  create_native_function(posix, "usleep", usleep);
#endif

  // errno
  object errno_obj = create_object();
  function get_errno = create_native_function(errno_obj, "get", &errno_getter);
  function set_errno = create_native_function(errno_obj, "set", &errno_setter);
  property_attributes errno_attr(permanent_shared_property,
                                 get_errno, set_errno);
  posix.define_property("errno", errno, errno_attr);
  create_native_function(posix, "strerror", &strerror_);
  create_native_function(posix, "perror", &std::perror);
}


}

