// -*- mode:c++;coding:utf-8; -*- vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Flusspferd contributors (see "CONTRIBUTORS" or
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

#ifndef FLUSSPFERD_PLUGINS_GMP_EXCEPTION_HPP
#define FLUSSPFERD_PLUGINS_GMP_EXCEPTION_HPP

#include "flusspferd/exception.hpp"
#include <stdexcept>

namespace multi_precision {
  struct exception : virtual flusspferd::exception {
    exception(char const *what, std::string const &type = "Error")
      : std::runtime_error(what), flusspferd::exception(what, type)
    { }
    exception(std::string const &what, std::string const &type = "Error")
      : std::runtime_error(what), flusspferd::exception(what, type)
    { }
    ~exception() throw() { }
  };

  struct logic_error : virtual exception, virtual std::logic_error {
    logic_error(char const *what, std::string const &type = "Error")
      : std::runtime_error(what),
        flusspferd::exception(what, type),
        multi_precision::exception(what, type),
        std::logic_error(what)
    { }
    logic_error(std::string const &what, std::string const &type = "Error")
      : std::runtime_error(what),
        flusspferd::exception(what, type),
        multi_precision::exception(what, type),
        std::logic_error(what)
    { }
    ~logic_error() throw() { }
  };

  struct argument_error : virtual logic_error {
    argument_error(char const *what, std::string const &type = "Error")
      : std::runtime_error(what),
        flusspferd::exception(what, type),
        multi_precision::exception(what, type),
        std::logic_error(what),
        logic_error(what, type)
    { }
    argument_error(std::string const &what, std::string const &type = "Error")
      : std::runtime_error(what),
        flusspferd::exception(what, type),
        multi_precision::exception(what, type),
        std::logic_error(what),
        logic_error(what, type)
    { }
    ~argument_error() throw() { }
  };

  struct type_error : virtual logic_error {
    type_error(char const *what)
      : std::runtime_error(what),
        flusspferd::exception(what, "TypeError"),
        multi_precision::exception(what, "TypeError"),
        std::logic_error(what),
        logic_error(what, "TypeError")
    { }
    type_error(std::string const &what)
      : std::runtime_error(what),
        flusspferd::exception(what, "TypeError"),
        multi_precision::exception(what, "TypeError"),
        std::logic_error(what),
        logic_error(what, "TypeError")
    { }
    ~type_error() throw() { }
  };

  struct runtime_error : virtual exception, virtual std::runtime_error {
    runtime_error(char const *what, std::string const &type = "Error")
      : std::runtime_error(what),
        flusspferd::exception(what, type),
        multi_precision::exception(what, type)
    { }
    runtime_error(std::string const &what, std::string const &type = "Error")
      : std::runtime_error(what),
        flusspferd::exception(what, type),
        multi_precision::exception(what, type)
    { }
    ~runtime_error() throw() { }
  };
}

#endif
