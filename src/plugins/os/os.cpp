// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Rüdiger Sonderfeld

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

#ifndef WIN32
#define POSIX
#endif

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/security.hpp"

#include <cstdlib>

#ifdef POSIX
#include <time.h>
#include <cerrno>
#elif WIN32
#include <windows.h>
#else
#error "OS not supported by os plugin"
#endif

using namespace flusspferd;

namespace {
#ifdef POSIX
void sleep_(unsigned ms) {
  timespec ts;
  ts.tv_sec = ms/1000;
  ts.tv_nsec = (ms%1000) * 1000000; // convert ms to ns
  timespec rem;
  while(nanosleep(&ts, &rem) == -1 && errno == EINTR)
    ts = rem;
}
#elif WIN32
void sleep_(unsigned ms) {
  Sleep(ms);
}
#endif

extern "C" void flusspferd_load(object os) {
  create_native_function(os, "system", &std::system);
  create_native_function(os, "sleep", &sleep_);
}
}
