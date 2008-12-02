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

#ifndef FLUSSPFERD_ROOT_VALUE_HPP
#define FLUSSPFERD_ROOT_VALUE_HPP

#include <boost/noncopyable.hpp>

namespace flusspferd {

class context;
class value;
class object;
class string;
class function;

namespace detail {

template<class T>
class root : public T, private boost::noncopyable {
public:
  root(T const &x = T());
  ~root();

  root &operator=(T const &o) {
    T::operator=(o);
    return *this;
  }

private:
  void *get_gcptr();
};

}

typedef detail::root<value> root_value;
typedef detail::root<object> root_object;
typedef detail::root<string> root_string;
typedef detail::root<function> root_function;

}

#endif
