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

#ifndef FLUSSPFERD_OBJECT_HPP
#define FLUSSPFERD_OBJECT_HPP

#ifndef PREPROC_DEBUG
#include "implementation/object.hpp"
#include "arguments.hpp"
#include "value.hpp"
#include "convert.hpp"
#include <string>
#include <memory>
#endif
#include <boost/preprocessor.hpp>

#ifndef FLUSSPFERD_PARAM_LIMIT
#define FLUSSPFERD_PARAM_LIMIT 5
#endif

namespace flusspferd {

#ifndef PREPROC_DEBUG
class value;
class context;
class function;
class native_object_base;
class property_iterator;
template<typename> class convert;
#endif

class object : public Impl::object_impl {
public:
#ifndef PREPROC_DEBUG
  object();
  object(Impl::object_impl const &o)
    : Impl::object_impl(o)
  { }
  ~object();

  bool is_null() const;

  bool is_array() const;

  void seal(bool deep);

  object parent();
  object prototype();

  void set_parent(object const &parent);
  void set_prototype(object const &prototype);
#endif

  value apply(object const &fn, arguments const &arg);

  value call(char const *name, arguments const &arg);
  value call(std::string const &name, arguments const &arg);

  value call(object obj, arguments const &arg);
  value call(arguments const &arg = arguments());

#define FLUSSPFERD_CALL_N(z, n, d) \
  FLUSSPFERD_CALL_N_2( \
    n, \
    BOOST_PP_TUPLE_ELEM(2, 0, d), \
    BOOST_PP_TUPLE_ELEM(2, 1, d)) \
  /**/

#define FLUSSPFERD_CALL_N_2(n, f_name, arg_type) \
  BOOST_PP_IF(n, template<, ) \
  BOOST_PP_ENUM_PARAMS(n, typename T) \
  BOOST_PP_IF(n, >, ) \
  value f_name( \
    arg_type x \
    BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n, T, const &arg)) \
  { \
    arguments arg; \
    BOOST_PP_REPEAT(n, FLUSSPFERD_CALL_ADD_PARAM, ~) \
    return f_name(x, arg); \
  } \
  /**/

#define FLUSSPFERD_CALL_ADD_PARAM(z, n, d) \
  typename convert<BOOST_PP_CAT(T, n) const &>::to_value BOOST_PP_CAT(c, n); \
  arg.push_root(BOOST_PP_CAT(c, n).perform(BOOST_PP_CAT(arg, n))); \
  /**/

#define FLUSSPFERD_CALLS(name, arg_type) \
  BOOST_PP_REPEAT( \
    BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT), \
    FLUSSPFERD_CALL_N, \
    (name, arg_type))

FLUSSPFERD_CALLS(apply, object const &)
FLUSSPFERD_CALLS(call, char const *)
FLUSSPFERD_CALLS(call, std::string const &)
FLUSSPFERD_CALLS(call, object const &)

#ifndef PREPROC_DEBUG
  enum property_flag {
    dont_enumerate = 1,
    read_only_property = 2,
    permanent_property = 4,
    shared_property = 8,
    permanent_shared_property = 12
  };

  struct property_attributes {
    unsigned flags;
    boost::optional<function const &> getter;
    boost::optional<function const &> setter;

    property_attributes();
    property_attributes(unsigned flags, 
      boost::optional<function const &> getter = boost::none,
      boost::optional<function const &> setter = boost::none);
  };

  void define_property(string const &name,
    value const &init_value = value(),
    property_attributes const attrs = property_attributes());

  void define_property(std::string const &name,
    value const &init_value = value(),
    property_attributes const attrs = property_attributes());

  void define_property(char const *name,
    value const &init_value = value(),
    property_attributes const attrs = property_attributes());

  bool get_property_attributes(char const *name, property_attributes &attrs);
  bool get_property_attributes(std::string name, property_attributes &attrs);
  bool get_property_attributes(string const &id, property_attributes &attrs);

  void set_property(char const *name, value const &v);
  void set_property(std::string const &name, value const &v);
  void set_property(value const &id, value const &v);

  value get_property(char const *name) const;
  value get_property(std::string const &name) const;
  value get_property(value const &id) const;
    
  bool has_property(char const *name) const;
  bool has_property(std::string const &name) const;
  bool has_property(value const &id) const;

  bool has_own_property(char const *name) const;
  bool has_own_property(std::string const &name) const;
  bool has_own_property(value const &id) const;

  void delete_property(char const *name);
  void delete_property(std::string const &name);
  void delete_property(value const &id);

  property_iterator begin() const;
  property_iterator end() const;
#endif
};

#ifndef PREPROC_DEBUG
inline bool operator==(object const &a, object const &b) {
  return Impl::operator==(a, b);
}

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
