// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Ash Berlin

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
#else ifdef HAVE_SLEEPEX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace flusspferd;

namespace {

#ifdef HAVE_SLEEPEX
int usleep(unsigned int usec) {
  // Windows only has microsecond sleep
  return SleepEx( (usec+500)/1000);
}
#endif



// import hook
extern "C" value flusspferd_load(object container)
{
  local_root_scope scope;
  object posix = create_object();
  
  container.set_property("posix", posix);

#ifdef HAVE_FORK
  // TODO: Need to work out how to make this play nice with the interactive repl
  posix.set_property("fork", create_native_function(fork, "fork"));
#endif
  posix.set_property("sleep", create_native_function(sleep, "sleep"));
  posix.set_property("usleep", create_native_function(usleep, "usleep"));

  return posix;
}


}

