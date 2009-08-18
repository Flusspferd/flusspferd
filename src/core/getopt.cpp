// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#include "flusspferd/getopt.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/array.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/property_iterator.hpp"
#include <iostream>
#include <map>

using namespace flusspferd;

void flusspferd::load_getopt_module(object container) {
  object exports = container.get_property_object("exports");

  flusspferd::create_native_function(exports, "getopt", &flusspferd::getopt);
}

struct optspec {
  std::map<std::string, int> options;

  optspec(object const &spec) {
    for (property_iterator it = spec.begin(); it != spec.end(); ++it) {
      std::string name = it->to_std_string();

      int data = 0;

      options.insert(std::make_pair(name, data));
      std::cout << name << std::endl;

      object item = spec.get_property_object(name);

      if (!item.is_null()) {
        value aliases = item.get_property("alias");
        if (aliases.is_undefined_or_null())
          aliases = item.get_property("aliases");

        if (!aliases.is_object() || !aliases.get_object().is_array()) {
          options.insert(std::make_pair(aliases.to_std_string(), data));
        } else {
          array aliases_a(aliases.get_object());

        }
      }
    }
  }
};

object flusspferd::getopt(
  object spec_, boost::optional<array const &> const &arguments_)
{
  if (!arguments_) {
    object sys = flusspferd::global().call("require", "system").to_object();
    array args(sys.get_property_object("args"));
    return getopt(spec_, args);
  }

  array const &arguments = arguments_.get();

  optspec spec(spec_);

  // TODO

  return flusspferd::object();
}
