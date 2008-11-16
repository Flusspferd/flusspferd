// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_EXCEPTION_HPP
#define FLUSSPFERD_EXCEPTION_HPP

#include <boost/shared_ptr.hpp>
#include <stdexcept>

namespace flusspferd {
  class exception : public std::runtime_error {
  public:
    exception(std::string const &what);
    ~exception() throw();

  public:
    void throw_js_INTERNAL();

  private:
    class impl;
    boost::shared_ptr<impl> p;
  };
}

#define FLUSSPFERD_CALLBACK_BEGIN try

#define FLUSSPFERD_CALLBACK_END \
    catch (::flusspferd::exception &e) { \
      e.throw_js_INTERNAL(); \
      return JS_FALSE; \
    } catch (::std::exception &e) { \
      ::flusspferd::exception x(e.what()); \
      x.throw_js_INTERNAL(); \
      return JS_FALSE; \
    } \
    return JS_TRUE

#endif /* FLUSSPFERD_EXCEPTION_HPP */
