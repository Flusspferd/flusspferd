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

#include "implementation/object.hpp"
#include "arguments.hpp"
#include "value.hpp"
#include "convert.hpp"
#include <string>
#include <memory>

namespace flusspferd {

class value;
class context;
class function;
class native_object_base;
class property_iterator;

class object : public Impl::object_impl {
public:
  object();
  object(Impl::object_impl const &o)
    : Impl::object_impl(o)
  { }
  ~object();

  bool is_valid() const;

  object get_parent();
  object get_prototype();

  void set_parent(object const &parent);
  void set_prototype(object const &prototype);

  value apply(object const &fn, arguments const &arg = arguments());

  value call(char const *name, arguments const &arg = arguments());
  value call(std::string const &name, arguments const &arg = arguments());

  value call(object obj, arguments const &arg = arguments());
  value call(arguments const &arg = arguments());

  enum property_flag {
    dont_enumerate = 1,
    read_only_property = 2,
    permanent_property = 4,
    shared_property = 8,
    permanent_shared_property = 12
  };

  void define_property(string const &name,
                       value const &init_value = value(),
                       unsigned flags = 0,
                       boost::optional<function const &> getter = boost::none,
                       boost::optional<function const &> setter = boost::none);

  void define_property(std::string const &name,
                       value const &init_value = value(),
                       unsigned flags = 0,
                       boost::optional<function const &> getter = boost::none,
                       boost::optional<function const &> setter = boost::none);

  void define_property(char const *name,
                       value const &init_value = value(),
                       unsigned flags = 0,
                       boost::optional<function const &> getter = boost::none,
                       boost::optional<function const &> setter = boost::none);

  void set_property(char const *name, value const &v);
  void set_property(std::string const &name, value const &v);
  void set_property(value const &id, value const &v);

  value get_property(char const *name) const;
  value get_property(std::string const &name) const;
  value get_property(value const &id) const;
    
  bool has_property(char const *name) const;
  bool has_property(std::string const &name) const;
  bool has_property(value const &id) const;

  void delete_property(char const *name);
  void delete_property(std::string const &name);
  void delete_property(value const &id);

  property_iterator begin() const;
  property_iterator end() const;
};

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

}

#endif /* FLUSSPFERD_OBJECT_HPP */
