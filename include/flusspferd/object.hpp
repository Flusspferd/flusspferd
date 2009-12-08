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

#ifndef FLUSSPFERD_OBJECT_HPP
#define FLUSSPFERD_OBJECT_HPP

#ifndef PREPROC_DEBUG
#include "spidermonkey/object.hpp"
#include "property_attributes.hpp"
#include "arguments.hpp"
#include "value.hpp"
#include "convert.hpp"
#include "string.hpp"
#include <string>
#include <memory>
#endif
#include "detail/limit.hpp"
#include <boost/preprocessor.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

namespace flusspferd {

#ifndef PREPROC_DEBUG
class value;
class context;
class function;
class native_object_base;
class property_iterator;
template<typename> class convert;
#endif

/**
 * A Javascript object.
 *
 * @see flusspferd::create_object()
 *
 * @ingroup value_types
 * @ingroup property_types
 */
class object : public Impl::object_impl {
public:
#ifndef PREPROC_DEBUG

  /**
   * Construct a new <code>null</code> object.
   *
   * Use flusspferd::create_object() to create a simple object.
   */
  object();

#ifndef IN_DOXYGEN
  object(Impl::object_impl const &o)
    : Impl::object_impl(o)
  { }
#endif

  /// Destructor.
  ~object();

  /**
   * Check if the object is null.
   *
   * @return Whether the object is null.
   */
  bool is_null() const;

  /**
   * Check if the object is an array.
   *
   * @return Whether the object is an array.
   */
  bool is_array() const;

  /**
   * Check if the object is a generator (i.e. return from a function which uses
   * yield). Due to limitations in the current Spidermonkey API there is a very
   * small chance this cant return true erroneously, but only if someone has
   * gone out of their way to make it happen.
   *
   * @return Whether the object is a generator.
   */
  bool is_generator() const;

  /**
   * Check if an object is an instance of the given constructor.
   *
   * @param constructor The constructor (not a string!).
   * @return Whether the object is an instance of the constructor.
   */
  bool instance_of(value constructor) const;

  /**
   * Seal the object.
   *
   * @param deep Whether to seal all reachable sub-objects, too.
   */
  void seal(bool deep);

  /// Get the object's parent (__parent__).
  object parent() const;
  
  /// Get the object's prototype (__proto__).
  object prototype() const;

  /// Get the object's constructor.
  object constructor() const;

  /**
   * Set the object's parent (__parent__).
   *
   * @param parent The object to set the parent to.
   */
  void set_parent(object const &parent);

  /**
   * Set the object's prototype (__proto__).
   *
   * @param prototype The object to set the prototype to.
   */
  void set_prototype(object const &prototype);
#endif

  /**
   * @name Method / function invocation
   *
   * Calling methods and functions.
   */
//@{

  /**
   * Apply a function to this object.
   *
   * @param fn The function to apply to this object.
   * @param arg The function arguments.
   * @return The function's return value.
   */
  value apply(object const &fn, arguments const &arg);

  /**
   * Call an object method.
   *
   * @param name The method name.
   * @param arg The %function %arguments.
   * @return The method's return value.
   */
  value call(char const *name, arguments const &arg);

  /**
   * Call an object method.
   *
   * @param name The method name.
   * @param arg The %function %arguments.
   * @return The method's return value.
   */
  value call(std::string const &name, arguments const &arg);

  /**
   * Call this object as a %function and apply it to @p obj.
   *
   * @param obj The object to apply this %function to.
   * @param arg The %function %arguments.
   * @return The %function's return value.
   */
  value call(object obj, arguments const &arg);

  /**
   * Call this object as a function on the global object.
   *
   * @param arg The %function %arguments.
   * @return The %function's return value.
   */
  value call(arguments const &arg = arguments());

#ifndef IN_DOXYGEN

#define FLUSSPFERD_CALL_N(z, n, d) \
  FLUSSPFERD_CALL_N_2( \
    n, \
    BOOST_PP_TUPLE_ELEM(2, 0, d), \
    BOOST_PP_TUPLE_ELEM(2, 1, d)) \
  /* */

#define FLUSSPFERD_CALL_N_2(n, f_name, arg_type) \
  BOOST_PP_EXPR_IF(n, template<) \
  BOOST_PP_ENUM_PARAMS(n, typename T) \
  BOOST_PP_EXPR_IF(n, >) \
  value f_name( \
    arg_type x \
    BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n, T, const &arg)) \
  { \
    arguments arg; \
    BOOST_PP_REPEAT(n, FLUSSPFERD_CALL_ADD_PARAM, ~) \
    return f_name(x, arg); \
  } \
  /* */

#define FLUSSPFERD_CALL_ADD_PARAM(z, n, d) \
  typename convert<BOOST_PP_CAT(T, n) const &>::to_value BOOST_PP_CAT(c, n); \
  arg.push_root(BOOST_PP_CAT(c, n).perform(BOOST_PP_CAT(arg, n))); \
  /* */

#define FLUSSPFERD_CALLS(name, arg_type) \
  BOOST_PP_REPEAT( \
    BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT), \
    FLUSSPFERD_CALL_N, \
    (name, arg_type))

FLUSSPFERD_CALLS(apply, object const &)
FLUSSPFERD_CALLS(call, char const *)
FLUSSPFERD_CALLS(call, std::string const &)
FLUSSPFERD_CALLS(call, object const &)

#else // IN_DOXYGEN
  /**
   * Apply a %function to this object.
   *
   * @param fn The %function to apply to this object.
   * @param ... The %function %arguments.
   * @return The function's return value.
   */
  value apply(object const &fn, ...);

  /**
   * Call an object method.
   *
   * @param name The method name.
   * @param ... The %function %arguments.
   * @return The method's return value.
   */
  value call(char const *name, ...);

  /**
   * Call an object method.
   *
   * @param name The method name.
   * @param ... The %function %arguments.
   * @return The method's return value.
   */
  value call(std::string const &name, ...);

  /**
   * Call this object as a %function and apply it to @p obj.
   *
   * @param obj The object to apply this %function to.
   * @param ... The %function %arguments.
   * @return The function's return value.
   */
  value call(object const &obj, ...);
#endif
//@}

#ifndef PREPROC_DEBUG

  /**
   * @name Properties
   *
   * Accessing properties and their attributes.
   */
//@{
  /**
   * Define a property.
   *
   * @param name The property's name.
   * @param init_value The initial value.
   * @param attrs The property's attributes.
   */
  void define_property(
    string const &name,
    value const &init_value = value(),
    property_attributes const &attrs = property_attributes());

  /**
   * Define a property.
   *
   * @param id The property's ID.
   * @param init_value The initial value.
   * @param attrs The property's attributes.
   */
  void define_property(
    value const &id,
    value const &init_value = value(),
    property_attributes const &attrs = property_attributes());

  /**
   * Define a property.
   *
   * The property will be initialised with <code>undefined</code>.
   *
   * @param name The property's name.
   * @param attrs The property's attributes.
   */
  void define_property(
    string const &name,
    property_attributes const &attrs)
  {
    return define_property(name, value(), attrs);
  }

private:
  class define_property_helper {
    object &obj;
    property_attributes attr;

  public:
    define_property_helper(object &obj, property_attributes const &attr)
      : obj(obj), attr(attr)
    {}

    template<typename T>
    define_property_helper operator()(T const &id) {
      obj.define_property(id, attr);
      return *this;
    }

    template<typename T, typename U>
    define_property_helper operator()(T const &id, U const &v) {
      obj.define_property(id, flusspferd::value(v), attr);
      return *this;
    }
  };

public:
  /**
   * Define multiple properties.
   *
   * Example:
   * @code
   obj.define_properties(read_only_property)("name1", value1)("name2", value2)("name3");
   @endcode
   *
   * @param attr The property attributes.
   */
  define_property_helper define_properties(property_attributes const &attr = property_attributes()) {
    return define_property_helper(*this, attr);
  }

  /**
   * Get a property's attributes.
   *
   * @param id The property's name / ID.
   * @return The attributes or boost::none if property does not exist.
   */
  boost::optional<property_attributes>
  get_property_attributes(string const &id) const;

  /**
   * Get a property's attributes.
   *
   * @param id The property's ID.
   * @param[out] attrs The property's attributes.
   * @return The attributes or boost::none if property does not exist.
   */
  boost::optional<property_attributes>
  get_property_attributes(value const &id) const;

  /**
   * Set a property.
   *
   * @param name The property's name.
   * @param v The new value.
   * @return @p v
   */
  value set_property(char const *name, value const &v);

  /**
   * Set a property.
   *
   * @param name The property's name.
   * @param v The new value.
   * @return @p v
   */
  value set_property(std::string const &name, value const &v);

  /**
   * Set a property.
   *
   * @param id The property's name / ID.
   * @param v The new value.
   * @return @p v
   */
  value set_property(value const &id, value const &v);

  /**
   * Set a property.
   *
   * @param id The property's name / ID.
   * @param v The new value.
   * @return @p v
   */
  template<typename T, typename U>
  value set_property(T const &id, U const &v
#ifndef IN_DOXYGEN
  , typename boost::disable_if<boost::is_same<U, value> >::type * = 0
#endif
  ) {
    return set_property(id, value(v));
  }

private:
  struct set_property_helper {
    object &obj;

    set_property_helper(object &obj) : obj(obj) {}

    template<typename T, typename U>
    set_property_helper operator()(T const &id, U const &v) {
      obj.set_property(id, v);
      return *this;
    }
  };

public:
  /**
   * Set multiple properties.
   *
   * Example:
   * @code
   obj.set_properties("name1", value1)("name2", value2);
   @endcode
   *
   * @param id The first property's name / ID.
   * @param v The new value for the first property.
   */
  template<typename T, typename U>
  set_property_helper set_properties(T const &id, U const &v) {
    return set_property_helper(*this)(id, v);
  }

  /**
   * Get a property.
   *
   * @param name The property's name.
   * @return The current value.
   */
  value get_property(char const *name) const;

  /**
   * Get a property.
   *
   * @param name The property's name.
   * @return The current value.
   */
  value get_property(std::string const &name) const;

  /**
   * Get a property.
   *
   * @param id The property's name / ID.
   * @return The current value.
   */
  value get_property(value const &id) const;

  /**
   * Get a property (as an object).
   *
   * @param id The property's name / ID.
   * @return The current value (as an object).
   */
  template<typename T>
  object get_property_object(T const &id) const {
    return get_property(id).to_object();
  }

  /**
   * Check whether a property exists on the object or any of its prototypes.
   *
   * @param name The property's name.
   * @return Whether the property exists.
   */
  bool has_property(char const *name) const;

  /**
   * Check whether a property exists on the object or any of its prototypes.
   *
   * @param name The property's name.
   * @return Whether the property exists.
   */
  bool has_property(std::string const &name) const;

  /**
   * Check whether a property exists on the object or any of its prototypes.
   *
   * @param id The property's name / ID.
   * @return Whether the property exists.
   */
  bool has_property(value const &id) const;

  /**
   * Check whether a property exists directly on the object.
   *
   * @param name The property's name.
   * @return Whether the property exists.
   */
  bool has_own_property(char const *name) const;

  /**
   * Check whether a property exists directly on the object.
   *
   * @param name The property's name.
   * @return Whether the property exists.
   */
  bool has_own_property(std::string const &name) const;

  /**
   * Check whether a property exists directly on the object.
   *
   * @param id The property's name / ID.
   * @return Whether the property exists.
   */
  bool has_own_property(value const &id) const;

  /**
   * Delete a property from the object.
   *
   * @param name The property's name.
   */
  void delete_property(char const *name);

  /**
   * Delete a property from the object.
   *
   * @param name The property's name.
   */
  void delete_property(std::string const &name);

  /**
   * Delete a property from the object.
   *
   * @param id The property's name / ID.
   */
  void delete_property(value const &id);

  /**
   * Return a property_iterator to the first property (in arbitrary order).
   *
   * @return The property_iterator to the first property.
   */
  property_iterator begin() const;

  /**
   * Return a property_iterator to behind the last property 
   * (in arbitrary order).
   *
   * @return The property_iterator to behind the last property.
   */
  property_iterator end() const;
//@}
#endif
};

#ifndef PREPROC_DEBUG
/**
 * Compare two object%s for equality.
 *
 * @relates object
 */
inline bool operator==(object const &a, object const &b) {
  return Impl::operator==(a, b);
}

/**
 * Compare two object%s for inequality.
 *
 * @relates object
 */
inline bool operator!=(object const &a, object const &b) {
  return Impl::operator!=(a, b);
}

template<>
struct detail::convert<object>
{
  typedef to_value_helper<object> to_value;

  struct from_value {
    root_value root;

    object perform(value const &v) {
      object o = v.to_object();
      root = o;
      return o;
    }
  };
};
#endif

}

#endif /* FLUSSPFERD_OBJECT_HPP */
