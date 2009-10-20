// -*- mode:c++;coding:utf-8; -*- vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:enc=utf-8:
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

#include "flusspferd.hpp"
#include "flusspferd/io/filesystem-base.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/object.hpp"
#include <boost/spirit/home/phoenix/core.hpp>
#include <boost/spirit/home/phoenix/bind.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <string>
#include <list>

#ifdef HAVE_EDITLINE
#include <editline/readline.h>
#elif HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#define HAVE_EDITLINE
#endif

#ifndef HISTORY_FILE_DEFAULT
#define HISTORY_FILE_DEFAULT "~/.flusspferd-history"
#endif

#ifdef FLUSSPFERD_RELOCATABLE
#include <boost/filesystem.hpp>
#endif

namespace phoenix = boost::phoenix;
namespace args = phoenix::arg_names;

class flusspferd_repl {
  bool interactive;
  bool interactive_set;
  bool machine_mode;
  std::istream in;

  bool config_loaded;
  std::string config_file;

  flusspferd::context co;
  flusspferd::current_context_scope scope;

  bool running;
  int exit_code;

  std::string history_file;

  int argc;
  char ** argv;

  enum Type { File, Expression, IncludePath, Module, MainModule };

  std::list<std::pair<std::string, Type> > files;

  flusspferd::object option_spec();

  // Returns list of files / expressions to execute
  void parse_cmdline();
  void print_help(bool do_quit = false);
  void print_version();
  void print_man();
  void print_bash();
  void add_file(std::string const &path, Type type, bool del_interactive);
  void load_config();

  bool getline(std::string &source, const char* prompt = "> ");

  void quit(int code) {
    running = false;
    exit_code = code;
    throw flusspferd::js_quit();
  }

  void run_cmdline();
  void repl_loop();
public:
  flusspferd_repl(int argc, char** argv);

  int run();
};

flusspferd_repl::flusspferd_repl(int argc, char **argv)
  : interactive(true),
    interactive_set(false),
    machine_mode(false),
    //file("typein"),
    in(std::cin.rdbuf()),
    config_loaded(false),
    // Default - can be changed by -c cmd line option
    config_file(INSTALL_PREFIX "/etc/flusspferd/jsrepl.js"),
    co(flusspferd::context::create()),
    scope(flusspferd::current_context_scope(co)),
    running(false),
    exit_code(0),
    history_file(HISTORY_FILE_DEFAULT),
    argc(argc),
    argv(argv)
{
  flusspferd::object g = flusspferd::global();

  // g.prototype() is available everywhere

  flusspferd::security::create(g);

  flusspferd::load_core(g, argv[0]);

#ifdef FLUSSPFERD_RELOCATABLE
  // Change the config to use the relative version
  boost::filesystem::path p = g.call("require", "flusspferd")
                               .to_object()
                               .get_property("executableName")
                               .to_std_string();
  p.remove_filename();
  p /=  boost::filesystem::path(FLUSSPFERD_ETC_PATH)
    /   std::string("jsrepl.js");
  config_file = p.string();
#endif

  flusspferd::create_native_function<void (int)>(
    g, "quit",
    phoenix::bind(&flusspferd_repl::quit, this, args::arg1));

  flusspferd::create_native_function(g, "gc", &flusspferd::gc);

  flusspferd::gc();
}

int flusspferd_repl::run() {
  try {
    run_cmdline();
  } catch (flusspferd::js_quit&) {
    if (!interactive)
      throw;
  } catch (std::exception &e) {
    if (interactive)
      std::cerr << "ERROR: " << e.what() << '\n';
    else
      throw;
  }

  if (interactive)
    repl_loop();
  return exit_code;
}

void flusspferd_repl::run_cmdline() {
  parse_cmdline();

  if (!config_loaded)
    load_config();

  flusspferd::object require_obj =
    flusspferd::global()
      .get_property_object("require");

  flusspferd::require require =
      dynamic_cast<flusspferd::require&>(
        *flusspferd::native_function_base::get_native(require_obj)
      );

  flusspferd::object module_obj = require_obj.get_property_object("main");

  typedef std::list<std::pair<std::string, Type> >::const_iterator iter;
  for (iter i = files.begin(), e = files.end(); i != e; ++i) {
    switch (i->second) {
    case File:
      // TODO: Move this logic into modules.cpp and make it set the right values
      if (!module_obj.has_own_property("id")) {
        std::string id = "file://"
                       + flusspferd::io::fs_base::canonicalize(i->first).string();
        require.set_main_module(id);
        require_obj.set_property("id", id);
      }
      flusspferd::execute(i->first.c_str());
      break;
    case Expression:
      flusspferd::evaluate(i->first, "[command line]", 0);
      break;
    case IncludePath:
      require_obj.get_property_object("paths").call("unshift", i->first);
      break;
    case Module:
      require_obj.call(flusspferd::global(), i->first);
      break;
    case MainModule:
      // TODO: make the module aware that it is the main module
      std::cout << "Setting require.main.id\n";
      require.set_main_module(i->first);
      std::cout << "Running main module\n";
      require_obj.call(flusspferd::global(), i->first);
      break;
    }
  }
}

void flusspferd_repl::repl_loop() {

#ifdef HAVE_EDITLINE
  if (!machine_mode && !history_file.empty()) {
    if(history_file.size() > 1 && history_file[0] == '~') {
      char const *const HOME = std::getenv("HOME"); 
      if (HOME && *HOME) {
        history_file = std::string(HOME) + history_file.substr(1);
      }
      else {
        throw std::runtime_error("Couldn't open history file `" + history_file
                                 + "' because $HOME is not set");
      }
    }
    read_history(history_file.c_str());
  }
#endif

  std::string source;
  unsigned int line = 0;

  running = true;

  // Disable strict mode for repl since its really annoying there.
  co.set_strict(false);

  while (running && getline(source)) {
    unsigned int startline = ++line;

    for (;;) {
      if (flusspferd::is_compilable(source)) {
        break;
      }

      std::string appendix;
      getline(appendix, "? ");
      source += appendix;

      ++line;
    }

    try {
      flusspferd::value v = flusspferd::evaluate(source, "[typein]", startline);
      if (!v.is_undefined()) {
        std::cout << v.to_source() << '\n';
      }
    }
    catch(std::exception &e) {
      std::cerr << "ERROR: " << e.what() << '\n';
    }
    flusspferd::gc();
  }

#ifdef HAVE_EDITLINE
  if (!machine_mode && !history_file.empty())
    write_history(history_file.c_str());
#endif
}

void flusspferd_repl::print_help(bool do_quit) {
  std::cerr << "usage: " << argv[0] << " [option] ... [file | -] [arg] ...\n\nOptions\n"
            << flusspferd::getopt_help(option_spec());

  if (do_quit) {
    interactive = false;
    throw flusspferd::js_quit();
  }
}

void flusspferd_repl::print_man() {
  if (!interactive_set)
    interactive = false;
  std::cout << flusspferd::getopt_man(option_spec());
  throw flusspferd::js_quit();
}

void flusspferd_repl::print_bash() {
  if (!interactive_set)
    interactive = false;
  std::cout << flusspferd::getopt_bash(option_spec());
  throw flusspferd::js_quit();
}

void flusspferd_repl::print_version() {
  if (!interactive_set)
    interactive = false;
  std::cout << "flusspferd shell version: " << FLUSSPFERD_VERSION << '\n';
  std::cout << "flusspferd library version: " << flusspferd::version() << '\n';
  std::cout.flush();
  throw flusspferd::js_quit();
}

void flusspferd_repl::add_file(
    std::string const &file, Type type, bool del_interactive)
{
  if (del_interactive && !interactive_set)
    interactive = false;
  files.push_back(std::make_pair(file, type));
}

void flusspferd_repl::load_config() {
  // Define the prelude property so its not a strict warning to assign to it.
  co.global().set_property("prelude", flusspferd::value());

  flusspferd::execute(config_file.c_str());
  config_loaded = true;

  // Get the prelude and execute it too
  flusspferd::value prelude = co.global().get_property("prelude");
  co.global().delete_property("prelude");


  if (!prelude.is_undefined_or_null()) {
    flusspferd::execute( prelude.to_string().c_str() );
  }
}

flusspferd::object flusspferd_repl::option_spec() {
  flusspferd::root_object spec(flusspferd::create_object());

  flusspferd::object options(flusspferd::create_object());
  spec.set_property("[options]", options);
  options.set_property("stop-early", true);

  flusspferd::object help(flusspferd::create_object());
  spec.set_property("help", help);
  help.set_property("alias", "h");
  help.set_property("doc", "Displays this message.");
  flusspferd::create_native_function(help, "callback",
    phoenix::bind(&flusspferd_repl::print_help, this, true));

  flusspferd::object version(flusspferd::create_object());
  spec.set_property("version", version);
  version.set_property("alias", "v");
  version.set_property("doc", "Print version and exit.");
  flusspferd::create_native_function(version, "callback",
    phoenix::bind(&flusspferd_repl::print_version, this));

  flusspferd::object config(flusspferd::create_object());
  spec.set_property("config", config);
  config.set_property("alias", "c");
  config.set_property("doc", "Load config from file.");
  config.set_property("argument", "required");
  config.set_property("argument_type", "file");
  flusspferd::create_native_function(config, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::ref(config_file) = args::arg2
    ));

  flusspferd::object interactive_(flusspferd::create_object());
  spec.set_property("interactive", interactive_);
  interactive_.set_property("alias", "i");
  interactive_.set_property("doc", "Enter interactive mode (after files).");
  flusspferd::create_native_function(interactive_, "callback",
    boost::function<void ()>((
      phoenix::ref(interactive) = true,
      phoenix::ref(interactive_set) = true
    )));

  flusspferd::object machine_mode_(flusspferd::create_object());
  spec.set_property("machine-mode", machine_mode_);
  machine_mode_.set_property("alias", "0");
  machine_mode_.set_property("doc", "(Interactive) machine command mode (separator '\\0').");
  flusspferd::create_native_function(interactive_, "callback",
    boost::function<void ()>((
      phoenix::ref(machine_mode) = true,
      phoenix::ref(interactive) = true,
      phoenix::ref(interactive_set) = true
    )));

  flusspferd::object file(flusspferd::create_object());
  spec.set_property("file", file);
  file.set_property("alias", "f");
  file.set_property("doc", "Run this file before standard script handling.");
  file.set_property("argument", "required");
  file.set_property("argument_type", "file");
  flusspferd::create_native_function(file, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, File, true)
    ));

  flusspferd::object expression(flusspferd::create_object());
  spec.set_property("expression", expression);
  expression.set_property("alias", "e");
  expression.set_property("doc", "Evaluate the expression.");
  expression.set_property("argument", "required");
  expression.set_property("argument_type", "expr");
  flusspferd::create_native_function(expression, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, Expression, true)
    ));

  flusspferd::object include_path(flusspferd::create_object());
  spec.set_property("include-path", include_path);
  include_path.set_property("alias", "I");
  include_path.set_property("doc", "Add include path.");
  include_path.set_property("argument", "required");
  include_path.set_property("argument_type", "path");
  flusspferd::create_native_function(include_path, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, IncludePath, false)
    ));

  flusspferd::object module(flusspferd::create_object());
  spec.set_property("module", module);
  module.set_property("alias", "M");
  module.set_property("doc", "Load module.");
  module.set_property("argument", "required");
  module.set_property("argument_type", "module");
  flusspferd::create_native_function(module, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, Module, false)
    ));

  flusspferd::object main_module(flusspferd::create_object());
  spec.set_property("main", main_module);
  main_module.set_property("alias", "m");
  main_module.set_property("doc", "Load module as the main module.");
  main_module.set_property("argument", "required");
  main_module.set_property("argument_type", "module");
  flusspferd::create_native_function(main_module, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, MainModule, true)
    ));

  flusspferd::object no_global_history(flusspferd::create_object());
  spec.set_property("no-global-history", no_global_history);
  no_global_history.set_property("doc", "Do not use a global history in interactive mode.");
  flusspferd::create_native_function(no_global_history, "callback",
    boost::function<void()>(
      phoenix::ref(history_file) = std::string()
    ));

  flusspferd::object history_file_(flusspferd::create_object());
  spec.set_property("history-file", history_file_);
  history_file_.set_property("doc", "Sets history file (default: ~/.flusspferd-history)");
  history_file_.set_property("argument", "required");
  history_file_.set_property("argument_type", "file");
  flusspferd::create_native_function(history_file_, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::ref(history_file) = args::arg2
    ));

  // Hidden Options for Generator Purpose
  flusspferd::object man_gen_(flusspferd::create_object());
  spec.set_property("hidden-man", man_gen_);
  man_gen_.set_property("hidden", "true");
  flusspferd::create_native_function(man_gen_, "callback",
    phoenix::bind(&flusspferd_repl::print_man, this));

  flusspferd::object bash_gen_(flusspferd::create_object());
  spec.set_property("hidden-bash", bash_gen_);
  bash_gen_.set_property("hidden", "true");
  flusspferd::create_native_function(bash_gen_, "callback",
    phoenix::bind(&flusspferd_repl::print_bash, this));

  return spec;
}

void flusspferd_repl::parse_cmdline() {
  flusspferd::root_object spec(option_spec());

  flusspferd::array arguments(flusspferd::create_array());

  for (int i = 1; i < argc; ++i)
    arguments.call("push", std::string(argv[i]));

  flusspferd::root_object results(flusspferd::getopt(spec, arguments));

  if (!config_loaded)
    load_config();

  arguments = results.get_property_object("_");

  std::string file("-");

  if (arguments.size() > 0) {
    file = arguments.call("shift").to_std_string();
    // TODO: Maybe check if stdin is actualy connected to a terminal?
    if (file != "-") {
      files.push_back(std::make_pair(file, File));
      if (!interactive_set)
        interactive = false;
    } else {
      interactive = true;
    }
  }

  arguments.call("unshift", file);

  flusspferd::object sys =
    flusspferd::global()
      .call("require", "system").to_object();

  sys.define_property("args", arguments,
                      flusspferd::read_only_property |
                      flusspferd::permanent_property);
}

namespace {
  bool all_whitespace_p(char const *in) {
    assert(in);
    while(*in) {
      if(!std::isspace(*in)) {
        return false;
      }
      ++in;
    }
    return true;
  }
}

bool flusspferd_repl::getline(std::string &source, char const *prompt) {
  if (machine_mode) {
    return std::getline(in, source, '\0');
  }
  else
#ifdef HAVE_EDITLINE
  if (interactive) {
    char* linep = readline(prompt);
    if (!linep) {
      std::cout << std::endl;
      return false;
    }
    if (linep[0] != '\0' && !all_whitespace_p(linep))
        add_history(linep);
    source = linep;
    source += '\n';
    std::free(linep);
    return true;
  } 
  else
#endif
  {
    std::cout << prompt;
    return std::getline(in, source);
  }
}

int main(int argc, char **argv) {
  try {
    flusspferd::init::initialize();
    flusspferd_repl repl(argc, argv);
    return repl.run();
  } catch (flusspferd::js_quit&) {
  } catch (std::exception &e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    return 1;
  }
}
