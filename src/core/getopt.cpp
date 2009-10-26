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

#include "flusspferd/getopt.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/array.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/property_iterator.hpp"
#include "flusspferd/root.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <algorithm>
#include <map>

using namespace flusspferd;

void flusspferd::load_getopt_module(object container) {
  object exports = container.get_property_object("exports");

  flusspferd::create_native_function(exports, "getopt", &flusspferd::getopt);
  flusspferd::create_native_function(exports, "getopt_help", &flusspferd::getopt_help);
  flusspferd::create_native_function(exports, "getopt_man", &flusspferd::getopt_man);
  flusspferd::create_native_function(exports, "getopt_bash", &flusspferd::getopt_bash);
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
  bool stop_early;

  optspec(object const &spec, array const &arguments)
    : arguments(arguments), stop_early(false)
  {
    if (spec.is_null())
      throw exception("Getopt specification must be a valid object");

    if (spec.has_property("[options]")) {
      object options = spec.get_property_object("[options]");
      stop_early = options.get_property("stop-early").to_boolean();
    }

    for (property_iterator it = spec.begin(); it != spec.end(); ++it) {
      std::string name = it->to_std_string();

      if (name.empty() || name[0] == '[')
        break;

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
      result.set_property(name, flusspferd::create<array>());
    array arr(result.get_property_object(name));

    if (eq == std::string::npos) {
      if (data->argument == item_type::required) {
        if (++pos >= arguments.size())
          throw exception(
            ("No argument supplied for long option " + name).c_str());
        std::string const &arg = arguments.get_element(pos).to_std_string();
        arr.push(arg);
        if (!data->callback.is_null())
          data->callback.call(result, name, arg);
      } else {
        arr.push(value());
        if (!data->callback.is_null())
          data->callback.call(result, name);
      }
    } else {
      if (data->argument == item_type::none)
        throw exception(("No argument allowed for option " + name).c_str());
      std::string const &arg = opt.substr(eq + 1);
      arr.push(arg);
      if (!data->callback.is_null())
        data->callback.call(result, name, arg);
    }
  }

  void handle_short(std::string const &opt, std::size_t &pos) {
    for (std::size_t i = 0; i < opt.size(); ++i) {
      std::string name(1, opt[i]);
      item_pointer data = options[name];
      if (!data)
        throw exception(("Unknown option " + name).c_str());

      if (!result.has_property(name))
        result.set_property(name, flusspferd::create<array>());
      array arr(result.get_property_object(name));
      
      if (data->argument != item_type::none) {
        std::string arg;
        if (i + 1 < opt.size())
          arg = opt.substr(i + 1);
        else if (pos + 1 < arguments.size())
          arg = arguments.get_element(++pos).to_std_string();
        else
          throw exception(
              ("No argument supplied for short option " + name).c_str());
        arr.push(arg);
        if (!data->callback.is_null())
          data->callback.call(result, name, arg);
        break;
      } else {
        arr.push(value());
        if (!data->callback.is_null())
          data->callback.call(result, name);
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

  spec.result = create<object>();

  array result_arguments = flusspferd::create<array>();
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
      result_arguments.push(arg);
      if (spec.stop_early)
        accept_options = false;
    }
  }

  return spec.result;
}

namespace {
  std::string name_to_option(std::string const &name) {
    if(name.size() > 1) {
      return std::string("--") + name;
    }
    else if(name.size() == 0) {
      return "";
    }
    else {
      return std::string("-") + name;
    }
  }

  std::string option_specification_text(std::string const &name, std::string const &argument, std::string const &required_argument) {
    if (argument.empty())
      return name_to_option(name);
    else if (name.size() == 1)
      return name_to_option(name) + required_argument;
    else
      return name_to_option(name) + '=' + argument;
  }

}

string flusspferd::getopt_help(object spec) {
  // 0 - list of aliases, 1 - name + arg, 2 - docstring
  enum { ALIASES = 0, NAME = 1, DOC = 2 };
  typedef boost::tuple<std::string, std::string, std::string> option_t;

  typedef std::vector<option_t> options_t;
  options_t options;

  std::size_t longest_name = 0;

  for (property_iterator it = spec.begin(); it != spec.end(); ++it) {
    std::string name = it->to_std_string();
    object item = spec.get_property_object(name);

    if (name[0] != '[' && !item.is_null()) {
      if (item.has_property("hidden") && item.get_property("hidden").to_boolean()) {
        continue;
      }

      std::string argument_type;
      std::string argument;
      std::string required_argument;
      if (item.has_property("argument_type"))
        argument_type = item.get_property("argument_type").to_std_string();
      else
        argument_type = "arg";
      if (item.has_property("argument")) {
        std::string arg = item.get_property("argument").to_std_string();
        boost::algorithm::to_lower(arg);
        if (arg != "none") {
          required_argument = '<' + argument_type + '>';
          if (arg == "required")
            argument = required_argument;
          else if (arg == "optional")
            argument = '[' + argument_type + ']';
        }
      }

      std::string name_arg = "    " + option_specification_text(name, argument, required_argument);
      longest_name = std::max(longest_name, name_arg.size());

      value aliases = item.get_property("alias");
      if (aliases.is_undefined_or_null()) {
        aliases = item.get_property("aliases");
      }

      std::string alias;
      if (!aliases.is_undefined_or_null()) {
        if (!aliases.is_object() || !aliases.get_object().is_array()) {
          alias = "    " + option_specification_text(aliases.to_std_string(), argument, required_argument) + '\n';
        }
        else {
          array aliases_a(aliases.get_object());
          for (std::size_t i = 0; i < aliases_a.length(); ++i) {
            alias += "    " + option_specification_text(aliases_a.get_element(i).to_std_string(), argument, required_argument) + '\n';
          }
        }
      }

      options.push_back(boost::make_tuple(alias, name_arg,
                                          item.has_property("doc") ?
                                          item.get_property("doc").to_std_string() :
                                          "..."));
    }
  }
  options.push_back(boost::make_tuple("", "    --", "Stop processing options."));

  std::string ret;
  typedef options_t::const_iterator iterator;
  enum { space_between_doc = 2 };
  for (iterator i = options.begin(); i != options.end(); ++i) {
    ret += boost::get<ALIASES>(*i) + boost::get<NAME>(*i);
    std::fill_n(std::back_inserter(ret), longest_name - boost::get<NAME>(*i).size() + space_between_doc, ' ');
    ret += boost::get<DOC>(*i) + "\n\n";
  }

  return ret;
}

namespace {
  // replace \ with \\ .
  std::string escape_string(std::string const &in) {
    std::string ret;
    for(std::string::const_iterator i = in.begin(); i != in.end(); ++i) {
      if(*i != '\\') {
        ret += *i;
      }
      else {
        ret += "\\\\";
      }
    }
    return ret;
  }
}

string flusspferd::getopt_man(object spec) {
  std::string ret;
  for (property_iterator it = spec.begin(); it != spec.end(); ++it) {
    std::string name = it->to_std_string();
    object item = spec.get_property_object(name);

    if (name[0] != '[' && !item.is_null()) {
      if (item.has_property("hidden") && item.get_property("hidden").to_std_string() == "true") {
        continue;
      }
      ret += ".TP\n";

      value aliases = item.get_property("alias");
      if (aliases.is_undefined_or_null()) {
        aliases = item.get_property("aliases");
      }

      if (!aliases.is_undefined_or_null()) {
        if (!aliases.is_object() || !aliases.get_object().is_array()) {
          ret += "\\fB" + escape_string(name_to_option(aliases.to_std_string())) + "\\fR, ";
        }
        else {
          array aliases_a(aliases.get_object());
          for (std::size_t i = 0; i < aliases_a.length(); ++i) {
            ret += "\\fB" + escape_string(name_to_option(aliases_a.get_element(i).to_std_string())) + "\\fR, ";
          }
        }
      }

      ret += "\\fB" + name_to_option(name) + "\\fR";

      std::string argument;
      if (item.has_property("argument_type")) {
        argument = "\\fI" + escape_string(item.get_property("argument_type").to_std_string()) + "\\fR";
      }
      if (item.has_property("argument")) {
        std::string arg = item.get_property("argument").to_std_string();
        boost::algorithm::to_lower(arg);
        if (arg == "required" && argument.empty()) {
          argument = "\\fIarg\\fR";
        }
        else if (arg == "optional") {
          if (argument.empty()) {
            argument = "[\\fIarg\\fR]";
          }
          else {
            argument = '[' + argument + ']';
          }
        }
      }
      if (!argument.empty()) {
        ret += ' ' + argument;
      }

      ret += '\n'; 
      ret += item.has_property("doc") ?
        escape_string(item.get_property("doc").to_std_string()) :
        "...";
      ret += '\n';
    }
  }
  return ret;
}

namespace {
  std::string arg_handler(std::string const &type) {
    // this could be memory hungry. Better way?
    static boost::unordered_set<std::string> const options = boost::assign::list_of
      ("alias")("arrayvar")("binding")("builtin")("command")("directory")
      ("disabled")("enabled")("export")("file")("function")("group")
      ("helptopic")("hostname")("job")("keyword")("running")("service")
      ("setopt")("shopt")("signal")("stopped")("user")("variable")
      .to_container(options);
    if(options.find(type) != options.end()) {
      return "COMPREPLY=( $(compgen -A " + type + " -- ${cur}) )";
    }
    else {
      return "";
    }
  }
}

string flusspferd::getopt_bash(object spec) {
  std::string ret =
    "    local cur prev opts\n"
    "    COMPREPLY=()\n"
    "    cur=\"${COMP_WORDS[COMP_CWORD]}\"\n"
    "    prev=\"${COMP_WORDS[COMP_CWORD-1]}\"\n";

  std::string options;
  std::string argument_handling;

  for (property_iterator it = spec.begin(); it != spec.end(); ++it) {
    std::string name = it->to_std_string();
    object item = spec.get_property_object(name);

    if (name[0] != '[' && !item.is_null()) {
      if (item.has_property("hidden") && item.get_property("hidden").to_std_string() == "true") {
        continue;
      }
      std::string arg_handling = "            "; // tmp

      value aliases = item.get_property("alias");
      if (aliases.is_undefined_or_null()) {
        aliases = item.get_property("aliases");
      }

      if (!aliases.is_undefined_or_null()) {
        if (!aliases.is_object() || !aliases.get_object().is_array()) {
          std::string const option = name_to_option(aliases.to_std_string());
          options += option + ' ';
          arg_handling += option + '|';
        }
        else {
          array aliases_a(aliases.get_object());
          for (std::size_t i = 0; i < aliases_a.length(); ++i) {
            std::string const option = aliases_a.get_element(i).to_std_string();
            options += option + ' ';
            arg_handling += option + '|';
          }
        }
      }

      std::string const option = name_to_option(name);
      options += option + ' ';
      arg_handling += option + ")\n";

      if (item.has_property("argument_bash")) {
        argument_handling += arg_handling +
          "                " + item.get_property("argument_bash").to_std_string() +
          "\n                return 0\n                ;;\n";
      }
      else if (item.has_property("argument_type")) {
        std::string const handler = arg_handler(item.get_property("argument_type").to_std_string());
        if (!handler.empty()) {
          argument_handling += arg_handling +
            "                " + handler +
            "\n                return 0\n                ;;\n";
        }
      }
    }
  }

  ret += 
    "    opts=\"" + options + "\"\n\n" +
    "    if [[ ${cur} == -* ]] ; then\n"
    "        COMPREPLY=( $(compgen -W \"${opts}\" -- ${cur}) )\n"
    "        return 0\n"
    "    else\n"
    "        case \"$prev\" in\n" +
    argument_handling +
    "            *)\n"
    "                COMPREPLY=( $(compgen -o default -- ${cur}) )\n"
    "                return 0\n"
    "                ;;\n"
    "        esac\n"
    "    fi\n";

  return ret;
}

