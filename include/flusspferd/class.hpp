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

#ifndef FLUSSPFERD_CLASS_HPP
#define FLUSSPFERD_CLASS_HPP

#include "native_function_base.hpp"
#include "create.hpp"
#include "init.hpp"
#include "local_root_scope.hpp"
#include <boost/ref.hpp>

namespace flusspferd {

namespace detail {

template<typename T>
struct class_constructor : native_function_base {
  class_constructor(unsigned arity, char const *name)
    : native_function_base(arity, name)
  {}

  void call(call_context &x) {
    x.result = create_native_object<T>(
        x.function.get_property("prototype").to_object(),
        boost::ref(x));
  }
};

}

struct class_info {
  static std::size_t constructor_arity() {
    return 0;
  }

  static void augment_constructor(object const &) {
  }

  static object create_prototype() {
    return create_object();
  }
};

template<typename T>
object load_class(object container = global()) {
  std::size_t const arity = T::class_info::constructor_arity::value;
  char const *name = T::class_info::constructor_name();

  local_root_scope scope;

  value previous = container.get_property(name);

  if (previous.is_object())
    return previous.get_object();

  function constructor(
    create_native_function<detail::class_constructor<T> >(arity, name));

  object prototype = T::class_info::create_prototype();

  get_current_context().add_prototype<T>(prototype);

  constructor.define_property(
    "prototype",
    prototype,
    object::dont_enumerate);

  T::class_info::augment_constructor(object(constructor));

  container.define_property(
    name,
    constructor,
    object::dont_enumerate);

  return constructor;
}

template<typename T>
bool load_internal_class() {
  local_root_scope scope;

  context ctx = get_current_context();

  object proto = ctx.get_prototype<T>();
  
  if (proto.is_valid())
    return false;

  proto = T::class_info::create_prototype();

  ctx.add_prototype<T>(proto);

  return true;
}

}

#endif
