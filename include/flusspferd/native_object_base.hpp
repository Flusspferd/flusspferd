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
  struct class_info {
    static char const *full_name() { return ""; }
  };

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

  /**
   * Test if the Javascript object is associated with a native object
   *
   * @return true if the give object is native
   */
  static bool is_object_native(object const &o);

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
   * Virtual method invoked whenever the object is called as if it were a
   * function.
   *
   * Default implementation: throw an exception.
   *
   * @param x The call context.
   */
  virtual void self_call(call_context &x);

  /**
   * Virtual method invoked whenever the object has to be traced.
   *
   * Default implementation: stub.
   *
   * For each value that is an otherwise unreachable member of the object and
   * that should be protected from garbage collection, call @c trc("x", x).
   * The string "x" does not need to be unique, it's used for debugging
   * purposes only.
   *
   * @code
flusspferd::value v;

...

void trace(flusspferd::tracer &trc) {
    trc("v", v);
}
   @endcode
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

  /**
   * Possible modes for native_object_base::property_op.
   */
  enum property_mode {
    /**
     * Used just before a new property is added to an object.
     */
    property_add = 0,

    /**
     * Used during most property deletions, even when the object has no property
     * with the given name / index.
     *
     * Will <em>not</em> be used for permanent properties.
     */
    property_delete = -1,

    /**
     * Used as the default getter for new properties or for non-existing
     * properties.
     */
    property_get = 1,

    /**
     * Used as the default setter for new properties.
     */
    property_set = 2
  };

  /**
   * Virtual method invoked for property addition, deletion, read access and
   * write access.
   *
   * Default implementation: stub.
   *
   * @param mode The reason for invocation.
   * @param id The property name / index.
   * @param[in,out] data The old/new value of the property.
   *
   * @see property_mode
   */
  virtual void property_op(property_mode mode, value const &id, value &data);

private:
#ifndef IN_DOXYGEN
  static object do_create_object(object const &proto);
  static object do_create_enumerable_object(object const &proto);

  friend object detail::create_native_object(object const &proto);
  friend object detail::create_native_enumerable_object(object const &proto);

private:
  class impl;
  boost::scoped_ptr<impl> p;

  friend class impl;
#endif
};

template<typename T>
T &cast_to_derived(native_object_base &o) {
  T *ptr = dynamic_cast<T*>(&o);
  if (!ptr)
    throw exception("Could not convert native object to derived type");
  return *ptr;
}

/**
 * Gets @p o as native object of class @p T. If @p o is not an object, not
 * native or not of class @p T then an exception will be thrown.
 *
 * @param o object to check
 * @see is_native
 * @ingroup classes
 */
template<typename T>
T &get_native(object const &o) {
  return cast_to_derived<T>(native_object_base::get_native(o));
}

template<typename T>
bool is_derived(native_object_base &o) {
  return dynamic_cast<T*>(&o);
}

/**
 * Checks if @p o is a native object of class @p T.
 *
 * @code
flusspferd::object o = v.get_object();
if (flusspferd::is_native<flusspferd::binary>(o) {
  flusspferd::binary &b = flusspferd::get_native<flusspferd::binary>(o);
}
@endcode
 *
 * @param o object to check
 * @see get_native
 * @ingroup classes
 */
template<typename T>
bool is_native(object const &o) {
  if ( native_object_base::is_object_native(o) ) {
    return is_derived<T>(native_object_base::get_native(o));
  }
  return false;
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
