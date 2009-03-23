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

#ifndef FLUSSPFERD_NATIVE_OBJECT_BASE_HPP
#define FLUSSPFERD_NATIVE_OBJECT_BASE_HPP

#include "object.hpp"
#include "convert.hpp"
#include "function_adapter.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_member_function_pointer.hpp>
#include <boost/any.hpp>
#include <memory>
#include <functional>

namespace flusspferd {

#ifndef IN_DOXYGEN
struct call_context;
class tracer;

namespace detail {
  object create_native_object(object const &proto);
  object create_native_enumerable_object(object const &proto);
}
#endif

/**
 * Native object base.
 *
 * @ingroup classes
 */
class native_object_base : public object, private boost::noncopyable {
public:
  /// Destructor.
  virtual ~native_object_base() = 0;

  /**
   * Explicitly return the associated object.
   * 
   * @return The associated object.
   */
  object get_object() {
    return *static_cast<object*>(this);
  }

  /**
   * Get the native object associated with a Javascript object.
   *
   * @return A reference to the object.
   */
  static native_object_base &get_native(object const &o);

public:
  /**
   * Associate with an object if there is no association yet.
   *
   * Do not use directly.
   *
   * @param o The object to associate with.
   */
  void load_into(object const &o);

protected:
  /**
   * Constructor.
   *
   * Immediately associates with object @p o.
   *
   * Do not use with arbitrary objects.
   *
   * @param o The object to associate with.
   */
  native_object_base(object const &o);

protected:
  /**
   * Type for simple member function Javascript methods.
   */
  typedef void (native_object_base::*native_method_type)(call_context &);

  /**
   * Type for simple boost::function Javascript methods.
   */
  typedef boost::function<void (call_context &)> callback_type;

protected:
  /**
   * Add a native method to this object.
   *
   * Note that this adds the method directly to the object and not to its
   * prototype, which is more usual.
   *
   * @param name The name of the method.
   * @param arity The method's arity.
   *
   * @see create_native_method
   */
  void add_native_method(std::string const &name, unsigned arity = 0);

protected:
  /**
   * Register a callback to be called by the default native method call
   * implementation (#call_native_method).
   *
   * @param name The name of the method.
   * @param method The C++ method to be called.
   */
  void register_native_method(
      std::string const &name, native_method_type method);

  /**
   * Register a callback to be called by the default native method call
   * implementation (#call_native_method).
   *
   * @param name The name of the method.
   * @param method The C++ functor to be called.
   */
  void register_native_method_cb(
      std::string const &name, callback_type const &cb);

  /**
   * Register a callback to be called by the default native method call
   * implementation (#call_native_method).
   *
   * @param name The name of the method.
   * @param method The C++ method to be called.
   */
  template<typename T>
  void register_native_method(
    std::string const &name, void (T::*method)(call_context&))
  {
    register_native_method(name, native_method_type(method));
  }

  /**
   * Register a callback to be called by the default native method call
   * implementation (#call_native_method) with arbitrary %function signature.
   *
   * The callback @p cb has to be compatible with the signature of @p T.
   *
   * @param T The function signature.
   * @param Method Whether the first parameter should be a reference to
   *          Javascript 'this'.
   * @param name The name of the method.
   * @param cb The C++ functor to be called.
   */
  template<typename T, bool Method, typename X>
  void register_native_method_cb(std::string const &name, X const &cb) {
    boost::function<T> fun(cb);
    function_adapter<T, Method> adapter(fun);
    register_native_method_cb(name, adapter);
  }

  /**
   * Register a callback to be called by the default native method call
   * implementation (#call_native_method) with arbitrary %function signature.
   *
   * The signature is determined automatically.
   *
   * @param name The name of the method.
   * @param f The C++ member function to be called.
   */
  template<typename R, typename T>
  void register_native_method(std::string const &name, R T::*f) {
    function_adapter_memfn<R, T> adapter(f);
    register_native_method_cb(name, adapter);
  }

protected:
  /**
   * Create a native method for use on prototypes.
   *
   * @param name The name of the method.
   * @param arity The arity of the method.
   */
  static function create_native_method(
    std::string const &name, unsigned arity);

  /**
   * Create a native method for use on prototypes.
   *
   * The generated method will be added to @p container.
   *
   * @param container The object (typically a prototype) to add the method to.
   * @param name The name of the method.
   * @param arity The arity of the method.
   */
  static function create_native_method(
    object &container, std::string const &name, unsigned arity);

protected:
  /**
   * Virtual method invoked for each native method call.
   *
   * Default implementation:
   * @li Try to find a method registered with one of the register_native_method
   *   overloads.
   * @li If found, call it.
   * @li Otherwise, throw an exception.
   *
   * @param name The name of the method that has been invoked.
   * @param x The call_context with the method call information.
   * 
   * @see add_native_method, create_native_method
   */
  virtual void call_native_method(std::string const &name, call_context &x);

  /**
   * Virtual method invoked whenever the object has to be traced.
   *
   * Default implementation: stub.
   *
   * For each value that is an otherwise unreachable member of the object and
   * that should be protected from garbage collection, call @c trc(x).
   *
   * @param trc The tracer to be used.
   *
   * @see @ref gc
   */
  virtual void trace(tracer &trc);

protected:
  /**
   * Possible property access methods. Can be combined by bitwise or.
   */
  enum property_access {
    /**
     * The property access uses the @c . or @c [] operator:
     * @c obj.id or @c obj[id], not @c id.
     */
    property_qualified = 1,

    /**
     * The property appears on the left-hand side of an assignment.
     */
    property_assigning = 2,

    /**
     * The property is being used in code like "<code>if (o.p) ...</code>",
     * or a similar idiom where the apparent purpose of the property access is
     * to detect whether the property exists.
     */
    property_detecting = 4,

    /**
     * The property is being declared in a var, const, or function declaration. 
     */
    property_declaring = 8,

    /**
     * class name used when constructing. (???)
     */
    property_classname = 16
  };

  /**
   * Virtual method invoked when a property is <em>not</em> found on an object.
   *
   * Default implementation: stub that returns @c false.
   *
   * It can be used to implement lazy properties.
   *
   * If possible, @p id will be an integer, or a string otherwise.
   *
   * @param id The property name / index.
   * @param access Information about the kind of property access. A combination
   *           of #property_access values.
   */
  virtual bool property_resolve(value const &id, unsigned access);

  /**
   * Virtual method invoked to start enumerating properties.
   *
   * Will be called only if class_info::custom_enumerate is activated.
   *
   * Default implementation: return boost::any().
   *
   * @param[out] num_properties The number of properties, if that can be
   *                computed in advance. Otherwise should be set to zero.
   *                Pre-initialized to zero.
   * @return An opaque iterator for use by #enumerate_next.
   */
  virtual boost::any enumerate_start(int &num_properties);

  /**
   * Virtual method invoked to advance the enumeration and pull out the next
   * property name / index.
   *
   * Default implementation: return @c value().
   *
   * Should return @c value() (@c undefined) to stop the enumeration.
   *
   * @param[in,out] iter The opaque iterator.
   * @return The next property name / index.
   */
  virtual value enumerate_next(boost::any &iter);

  enum property_mode { 
    property_add = 0,
    property_delete = -1,
    property_get = 1,
    property_set = 2
  };

  virtual void property_op(property_mode mode, value const &id, value &data);

  typedef void (native_object_base::*property_callback)(
      property_mode mode, value &data);

  void add_property_op(std::string const &id, property_callback cb);

  template<typename T>
  void add_property_op(
      std::string const &id, void (T::*cb)(property_mode, value &))
  {
    add_property_op(id, property_callback(cb));
  }

  template<typename T>
  void define_native_property(
      std::string const &id,
      unsigned flags,
      void (T::*cb)(property_mode, value &))
  {
    add_property_op(id, cb);
    define_property(id, value(), flags);
  }

private:
  static object do_create_object(object const &proto);
  static object do_create_enumerable_object(object const &proto);

  friend object detail::create_native_object(object const &proto);
  friend object detail::create_native_enumerable_object(object const &proto);

private:
  class impl;
  boost::scoped_ptr<impl> p;

  friend class impl;
};

template<typename T>
T &cast_to_derived(native_object_base &o) {
  T *ptr = dynamic_cast<T*>(&o);
  if (!ptr)
    throw exception("Could not convert native object to derived type");
  return *ptr;
}

template<typename T>
T &get_native(object const &o) {
  return cast_to_derived<T>(native_object_base::get_native(o));
}

template<typename T>
struct detail::convert_ptr<T, native_object_base> {
  struct to_value {
    value perform(T *ptr) {
      if (!ptr)
        return object();
      return *static_cast<object const *>(ptr);
    }
  };

  struct from_value {
    T *perform(value const &v) {
      if (!v.is_object())
        throw exception("Value is no object");
      return &native_object_base::get_native(v.get_object());
    }
  };
};

namespace detail {

template<typename T, typename O>
struct convert_ptr<
  T, O,
  typename boost::enable_if<
    typename boost::is_base_of<native_object_base, O>::type
  >::type
>
{
  typedef typename convert_ptr<T, native_object_base>::to_value to_value;

  struct from_value {
    typename convert_ptr<native_object_base>::from_value base;

    T *perform(value const &v) {
      return &dynamic_cast<T&>(*base.perform(v));
    }
  };
};

}

}

#endif
