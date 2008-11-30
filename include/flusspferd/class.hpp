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
#include <boost/mpl/size_t.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/ref.hpp>

namespace flusspferd {

// Everything in this namespace is 'private'
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

template<typename T>
object load_class(object &container, char const *name) {
  context ctx = get_current_context();

  object constructor(ctx.get_constructor<T>());

  object prototype = T::class_info::create_prototype();

  ctx.add_prototype<T>(prototype);

  constructor.define_property(
    "prototype",
    prototype,
    object::dont_enumerate);

  T::class_info::augment_constructor(object(constructor));

  container.define_property(name, constructor, object::dont_enumerate);

  return constructor;
}

}

/**
 * Information about classes exposed to javascript. Expose a class to 
 * javascript, you will need to define a <class_info> struct as a public member
 * of your class, as shown in the example below
 *
 * @code
 * class MyClass : public native_object_base {
 * public:
 *   struct class_info : flusspferd::class_info {
 *     static char const *constructor_name() { return "MyJSClass"; }
 *     typedef boost:mpl:size_t<2> constructor_arity;
 *     static object create_prototype() { ... }
 *   };
 * };
 * @endcode
 *
 * Inheriting from <flusspferd::class_info> gives you default behaviour of 0
 * constructor arity, no augmentation and an empty prototype. You will always
 * have to define the C<constructor_name> yourself.
 */
struct class_info {
  typedef boost::mpl::bool_<true> constructible;

  /**
   * How many paremeters the constructor expects from JS.
   *
   * @code
   * typedef boost::mpl::size_t<2> constructor_arity;
   * @endcode
   *
   * for example will mean that your constructor will have a (nominal) arity 
   * of 2.
   */
  typedef boost::mpl::size_t<0> constructor_arity;

#ifdef IN_DOXYGEN
  /**
   * A function that returns the name of the constructor. 
   */
   static char const *constructor_name();
#endif

  /**
   * Function: augment_constructor 
   *
   * Hook to add properties to the constructor object. Most commonly used to
   * add static methods or properties
   */
  static void augment_constructor(object const &ctor) {
    (void) ctor;
  }

  /**
   * Function: create_prototype 
   *
   * Return the prototype for this class. Defaults to an empty object.
   */
  static object create_prototype() {
    return create_object();
  }
};

/**
 * Function: load_class
 *
 * Expose a class with a native constructor to Javascript.
 *
 * @param container  Object in which to define the constructor.
 *
 * The class name is pulled from the class's <class_info> struct.
*/
template<typename T>
object load_class(
  object container = global(),
  typename boost::enable_if<
    typename T::class_info::constructible
  >::type * = 0)
{
  std::size_t const arity = T::class_info::constructor_arity::value;
  char const *name = T::class_info::constructor_name();

  local_root_scope scope;

  context ctx = get_current_context();

  value previous = container.get_property(name);

  if (previous.is_object())
    return previous.get_object();

  if (!ctx.get_constructor<T>().is_valid())
    ctx.add_constructor<T>(
      create_native_function<detail::class_constructor<T> >(arity, name));

  return detail::load_class<T>(container, name);
}

/**
 * Expose a class without a native constructor to Javascript.
 *
 * @param container Object in which to define the constructor.
 *
 * Create a class/constructor on container without constructor.
*/
template<typename T>
bool load_class(
  object container = global(),
  typename boost::enable_if<
    boost::mpl::not_<typename T::class_info::constructible>
  >::type * = 0)
{
  char const *name = T::class_info::constructor_name();

  local_root_scope scope;

  context ctx = get_current_context();

  value previous = container.get_property(name);

  if (previous.is_object())
    return previous.get_object();

  if (!ctx.get_constructor<T>().is_valid())
    ctx.add_constructor<T>(create_object());

  return detail::load_class<T>(container, name);
}

}

#endif
