// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
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

#ifndef IN_DOXYGEN

namespace detail {

struct unconstructible_class_constructor : native_function_base {
  unconstructible_class_constructor(char const *name)
    : native_function_base(0, name)
  {}

  void call(call_context &);
};

template<typename T>
struct class_constructor : native_function_base {
  class_constructor(unsigned arity, char const *name)
    : native_function_base(arity, name)
  {}

  void call(call_context &x) {
    x.result = flusspferd::create_native_object<T>(
        x.function.get_property_object("prototype"),
        boost::ref(x));
  }
};

template<typename T>
void load_class(
    context &ctx, object &constructor) 
{
  object prototype = T::class_info::create_prototype();
  ctx.add_prototype<T>(prototype);

  prototype.define_property(
    "constructor",
    constructor,
    permanent_property | read_only_property | dont_enumerate);

  constructor.define_property(
    "prototype",
    prototype,
    dont_enumerate);

  T::class_info::augment_constructor(constructor);
}

}

#endif

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
 *     static char const *full_name() { return "MyNamespace.MyJSClass"; }
 *     typedef boost:mpl:size_t<2> constructor_arity;
 *     static object create_prototype() { ... }
 *   };
 * };
 * @endcode
 *
 * Inheriting from flusspferd::class_info gives you default behaviour of 0
 * constructor arity, no augmentation and an empty prototype. You will always
 * have to define the @c constructor_name and @c full_name yourself.
 *
 * @see FLUSSPFERD_CLASS_DESCRIPTION
 *
 * @ingroup classes
 */
struct class_info {
  /**
   * Whether the type is constructible.
   */
  typedef boost::mpl::bool_<true> constructible;

  /**
   * Whether the class overrides the standard enumerate hooks.
   *
   * Should be enabled when the class overrides
   * native_object_base::enumerate_start and native_object_base::enumerate_next.
   */
  typedef boost::mpl::bool_<false> custom_enumerate;

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

  /**
   * Hook to add properties to the constructor object. Most commonly used to
   * add static methods or properties
   *
   * @param ctor The constructor to be augmented.
   */
  static void augment_constructor(object &ctor) {
    (void) ctor;
  }

  /**
   * Hook to create the prototype object of the class.
   *
   * @return Return the newly created prototype object.
   */
  static object create_prototype() {
    return create_object();
  }
};

#ifndef IN_DOXYGEN

template<typename T>
object load_class(
  object container = global(),
  typename boost::enable_if<
    typename T::class_info::constructible
  >::type * = 0)
{
  std::size_t const arity = T::class_info::constructor_arity::value;
  char const *name = T::class_info::constructor_name();
  char const *full_name = T::class_info::full_name();

  local_root_scope scope;

  context ctx = current_context();

  value previous = container.get_property(name);

  if (previous.is_object())
    return previous.get_object();

  object constructor = ctx.constructor<T>();

  if (constructor.is_null()) {
    constructor =
      create_native_functor_function<
          detail::class_constructor<T> 
        >(flusspferd::object(), arity, full_name);
    ctx.add_constructor<T>(constructor);
    detail::load_class<T>(ctx, constructor);
  }

  container.define_property(name, constructor, dont_enumerate);

  return constructor;
}

template<typename T>
object load_class(
  object container = global(),
  typename boost::enable_if<
    boost::mpl::not_<typename T::class_info::constructible>
  >::type * = 0)
{
  char const *name = T::class_info::constructor_name();

  local_root_scope scope;

  context ctx = current_context();

  value previous = container.get_property(name);

  if (previous.is_object())
    return previous.get_object();

  object constructor = ctx.constructor<T>();

  if (constructor.is_null()) {
    char const *full_name = T::class_info::full_name();
    constructor =
      create_native_functor_function<
          detail::unconstructible_class_constructor
        >(flusspferd::object(), full_name);
    ctx.add_constructor<T>(constructor);
    detail::load_class<T>(ctx, constructor);
  }

  container.define_property(name, constructor, dont_enumerate);

  return constructor;
}

#else

/**
 * Expose a class to Javascript.
 *
 * If the class is not constructible, trying to create an instance of the class
 * will throw an error. A dummy "constructor" will be created though.
 *
 * The class name is pulled from the class's class_info struct, as
 * well as other information.
 *
 * @param container Object in which to define the constructor.
 *
 * @see flusspferd::class_info
 *
 * @ingroup classes
 */
template<typename T>
object load_class(object container = global());

#endif

}

#endif
