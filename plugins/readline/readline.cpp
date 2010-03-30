// -*-mode:c++;coding:utf-8;-*- vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2009 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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

#include "flusspferd/create.hpp"
#include "flusspferd/create_on.hpp"
#include "flusspferd/create/function.hpp"
#include "flusspferd/modules.hpp"

#ifdef HAVE_EDITLINE
#include <editline/readline.h>
#elif HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include <string>
#include <cstring>
#include <cstdlib>

namespace {
  flusspferd::value readline_(flusspferd::value prompt) {
    char *read;
    if(prompt.is_undefined_or_null()) {
      read = ::readline(0x0);
    }
    else {
      flusspferd::string const s = prompt.to_string();
      read = ::readline(s.c_str());
    }
    if(read) {
      std::string const ret = read;
      std::free(read);
      return flusspferd::value(ret);
    }
    return flusspferd::value();
  }

  void read_history_(flusspferd::value file) {    
    int err;
    if(file.is_undefined_or_null()) {
      err = ::read_history(0x0);
    }
    else {
      flusspferd::string const s = file.to_string();
      err = ::read_history(s.c_str());
    }
    if(err != 0) {
      throw flusspferd::exception(std::string("readline.readHistory: failed to read history `")
                                  + std::strerror(err) + "`");
    }
  }

  void write_history_(flusspferd::value file) {
    int err;
    if(file.is_undefined_or_null()) {
      err = ::write_history(0x0);
    }
    else {
      flusspferd::string const s = file.to_string();
      err = ::write_history(s.c_str());
    }
    if(err != 0) {
      throw flusspferd::exception(std::string("readline.writeHistory: failed to write history `")
                                  + std::strerror(err) + "`");
    }
  }
}

using namespace flusspferd;

FLUSSPFERD_LOADER_SIMPLE(readline) {
  create_on(readline)
    .create<function>("readline", &readline_)
    .create<function>("usingHistory", &::using_history)
    .create<function>("readHistory", &read_history_)
    .create<function>("addHistory", &::add_history)
    .create<function>("writeHistory", &write_history_);
}
