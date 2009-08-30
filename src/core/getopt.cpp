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
#include "flusspferd/root.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <map>

using namespace flusspferd;

void flusspferd::load_getopt_module(object container) {
  object exports = container.get_property_object("exports");

  flusspferd::create_native_function(exports, "getopt", &flusspferd::getopt);
}

namespace {
struct optspec {
  struct item_type {
    enum argument_type { required = 1, optional = 2, none = 0 };
    argument_type argument;
    root_function callback;

    item_type() : argument(none) {}
  };
  typedef boost::shared_ptr<item_type> item_pointer;
  typedef std::map<std::string, item_pointer> map_type;

  map_type options;
  array const &arguments;
  object result;

  optspec(object const &spec, array const &arguments)
    : arguments(arguments)
  {
    if (spec.is_null())
      throw exception("Getopt specification must be a valid object");

    for (property_iterator it = spec.begin(); it != spec.end(); ++it) {
      std::string name = it->to_std_string();

      item_pointer data(new item_type);

      options.insert(map_type::value_type(name, data));

      object item = spec.get_property_object(name);

      if (!item.is_null()) {
        value aliases = item.get_property("alias");
        if (aliases.is_undefined_or_null())
          aliases = item.get_property("aliases");

        if (!aliases.is_undefined_or_null()) {
          if (!aliases.is_object() || !aliases.get_object().is_array()) {
            options.insert(map_type::value_type(aliases.to_std_string(), data));
          } else {
            array aliases_a(aliases.get_object());
            for (std::size_t i = 0; i < aliases_a.length(); ++i)
              options.insert(map_type::value_type(
                aliases_a.get_element(i).to_std_string(), data));
          }
        }

        object callback = item.get_property_object("callback");
        if (!callback.is_null()) {
          data->callback = function(callback);
        }

        if (item.has_property("argument")) {
          std::string argument = item.get_property("argument").to_std_string();
          boost::algorithm::to_lower(argument);
          if (argument == "none")
            data->argument = item_type::none;
          else if (argument == "required")
            data->argument = item_type::required;
          else if (argument == "optional")
            data->argument = item_type::optional;
          else
            throw exception("Invalid argument type. Must be one of 'required',"
                            " 'optional' or 'none'.");
        }
      }
    }
  }

  void handle_option(std::string const &opt, std::size_t &pos) {
    std::size_t eq = opt.find('=');

    std::string name = opt.substr(0, eq);
    item_pointer data = options[name];
    if (!data)
      throw exception(("Unknown option " + name).c_str());

    if (!result.has_property(name))
      result.set_property(name, create_array());
    array arr(result.get_property_object(name));

    if (eq == std::string::npos)
      arr.call("push", value());
    else {
      if (data->argument == item_type::none)
        throw exception(("No argument allowed for option " + name).c_str());
      arr.call("push", opt.substr(eq + 1));
    }
  }

  void handle_short(std::string const &opt, std::size_t &pos) {
    for (std::size_t i = 0; i < opt.size(); ++i) {
      std::string name(1, opt[i]);
      item_pointer data = options[name];
      if (!data)
        throw exception(("Unknown option " + name).c_str());

      if (!result.has_property(name))
        result.set_property(name, create_array());
      array arr(result.get_property_object(name));
      
      if (data->argument != item_type::none) {
        if (i + 1 < opt.size())
          arr.call("push", opt.substr(i + 1));
        else if (pos + 1 < arguments.size())
          arr.call("push", arguments.get_element(++pos));
        else
          throw exception(
              ("No argument supplied for short option " + name).c_str());
        break;
      } else {
        arr.call("push", value());
      }
    }
  }
};
}

object flusspferd::getopt(
  object spec_, boost::optional<array const &> const &arguments_)
{
  if (!arguments_) {
    object sys = flusspferd::global().call("require", "system").to_object();
    array args(sys.get_property_object("args"));
    return getopt(spec_, args);
  }

  array const &arguments = arguments_.get();

  optspec spec(spec_, arguments);

  spec.result = create_object();

  array result_arguments = create_array();
  spec.result.set_property("_", result_arguments);

  bool accept_options = true;

  for (std::size_t i = 0; i < arguments.length(); ++i) {
    std::string arg = arguments.get_element(i).to_std_string();
    if (accept_options && arg.size() >= 2 && arg[0] == '-') {
      if (arg[1] == '-')
        if (arg.size() == 2)
          accept_options = false;
        else
          spec.handle_option(arg.substr(2), i);
      else
        spec.handle_short(arg.substr(1), i);
    } else {
      result_arguments.call("push", arg);
    }
  }

  return spec.result;
}
