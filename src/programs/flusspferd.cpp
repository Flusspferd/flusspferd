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

#include "flusspferd.hpp"
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
  void add_file(std::string const &path, Type type, bool del_interactive);
  void load_config();

  bool getline(std::string &source, const char* prompt = "> ");

  void quit(int code) {
    running = false;
    exit_code = code;
    throw flusspferd::js_quit();
  }

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

  flusspferd::security::create(g.prototype());

  flusspferd::load_core(g.prototype());

  flusspferd::create_native_function<void (int)>(
    g, "quit",
    phoenix::bind(&flusspferd_repl::quit, this, args::arg1));

  flusspferd::create_native_function(g, "gc", &flusspferd::gc);

  flusspferd::gc();
}

int flusspferd_repl::run() {
  parse_cmdline();

  if (!config_loaded)
    load_config();

  flusspferd::object require_obj =
      flusspferd::global()
        .prototype()
        .get_property_object("require");

  typedef std::list<std::pair<std::string, Type> >::const_iterator iter;
  for (iter i = files.begin(), e = files.end(); i != e; ++i) {
    switch (i->second) {
    case File:
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
      require_obj.call(flusspferd::global(), i->first);
      break;
    }
  }
  
  if (!interactive)
    return exit_code;

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

  while (running && getline(source)) {
    unsigned int startline = ++line;

    for (;;) {
      JSBool compilable =
          JS_BufferIsCompilableUnit(
            flusspferd::Impl::current_context(),
            flusspferd::Impl::get_object(flusspferd::global()),
            source.data(),
            source.size());

      if (compilable)
        break;

      std::string appendix;
      getline(appendix, "? ");
      source += appendix;

      ++line;
    }

    try {
      flusspferd::value v = flusspferd::evaluate(source, "[typein]", startline);
      if (!v.is_undefined())
        std::cout << v << '\n';
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

  return exit_code;
}

void flusspferd_repl::print_help(bool do_quit) {
  std::cerr << "usage: " << argv[0] << " [option] ... [file | -] [arg] ...\n\n"
"Options\n"
"    -h\n"
"    --help                   Displays this message.\n"
"\n"
"    -v\n"
"    --version                Print version and exit.\n"
"\n"
"    -c <file>\n"
"    --config <file>          Load config from file.\n"
"\n"
"    -i\n"
"    --interactive            Enter interactive mode (after files).\n"
"\n"
"    -0\n"
"    --machine-mode           (Interactive) machine command mode (separator\n"
"                             '\\0').\n"
"\n"
"    -e <expr>\n"
"    --expression <expr>      Evaluate the expression.\n"
"\n"
"    -f <file>\n"
"    --file <file>            Run this file before standard script handling.\n"
"\n"
"    -I <path>\n"
"    --include-path <path>    Add include path.\n"
"\n"
"    -M <module>\n"
"    --module <module>        Load module.\n"
"\n"
"    -m <module>\n"
"    --main <module>\n        Load module as the main module.\n"
"\n"
"    --no-global-history      Do not use a global history in interactive mode.\n"
"\n"
"    --history-file <file>    Sets history file (default: ~/.flusspferd-history)\n"
"\n"
"    --                       Stop processing options.\n\n";

  if (do_quit)
    throw flusspferd::js_quit();
}

void flusspferd_repl::print_version() {
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
  flusspferd::execute(config_file.c_str());
  config_loaded = true;

  // Get the prelude and execute it too
  flusspferd::value prelude = co.global().get_property("prelude");
  
  if (!prelude.is_undefined_or_null()) {
    flusspferd::execute(
      prelude.to_string().c_str(),
      flusspferd::global().prototype());
  }
}

flusspferd::object flusspferd_repl::option_spec() {
  flusspferd::root_object spec(flusspferd::create_object());

  flusspferd::object help(flusspferd::create_object());
  spec.set_property("help", help);
  help.set_property("alias", "h");
  flusspferd::create_native_function(help, "callback",
    phoenix::bind(&flusspferd_repl::print_help, this, true));

  flusspferd::object version(flusspferd::create_object());
  spec.set_property("version", version);
  version.set_property("alias", "v");
  flusspferd::create_native_function(version, "callback",
    phoenix::bind(&flusspferd_repl::print_version, this));

  flusspferd::object config(flusspferd::create_object());
  spec.set_property("config", config);
  config.set_property("alias", "c");
  config.set_property("argument", "required");
  flusspferd::create_native_function(config, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::ref(config_file) = args::arg2
    ));

  flusspferd::object interactive_(flusspferd::create_object());
  spec.set_property("interactive", interactive_);
  interactive_.set_property("alias", "i");
  flusspferd::create_native_function(interactive_, "callback",
    boost::function<void ()>((
      phoenix::ref(interactive) = true,
      phoenix::ref(interactive_set) = true
    )));

  flusspferd::object machine_mode_(flusspferd::create_object());
  spec.set_property("machine-mode", machine_mode_);
  machine_mode_.set_property("alias", "0");
  flusspferd::create_native_function(interactive_, "callback",
    boost::function<void ()>((
      phoenix::ref(machine_mode) = true,
      phoenix::ref(interactive) = true,
      phoenix::ref(interactive_set) = true
    )));

  flusspferd::object file(flusspferd::create_object());
  spec.set_property("file", file);
  file.set_property("alias", "f");
  file.set_property("argument", "required");
  flusspferd::create_native_function(file, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, File, true)
    ));

  flusspferd::object expression(flusspferd::create_object());
  spec.set_property("expression", expression);
  expression.set_property("alias", "e");
  expression.set_property("argument", "required");
  flusspferd::create_native_function(expression, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, Expression, true)
    ));

  flusspferd::object include_path(flusspferd::create_object());
  spec.set_property("include-path", include_path);
  include_path.set_property("alias", "I");
  include_path.set_property("argument", "required");
  flusspferd::create_native_function(include_path, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, IncludePath, false)
    ));

  flusspferd::object module(flusspferd::create_object());
  spec.set_property("module", module);
  module.set_property("alias", "M");
  module.set_property("argument", "required");
  flusspferd::create_native_function(module, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, Module, false)
    ));

  flusspferd::object main_module(flusspferd::create_object());
  spec.set_property("main", main_module);
  main_module.set_property("alias", "m");
  main_module.set_property("argument", "required");
  flusspferd::create_native_function(main_module, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::bind(&flusspferd_repl::add_file, this,
                    args::arg2, MainModule, true)
    ));

  flusspferd::object no_global_history(flusspferd::create_object());
  spec.set_property("no-global-history", no_global_history);
  flusspferd::create_native_function(no_global_history, "callback",
    boost::function<void()>(
      phoenix::ref(history_file) = std::string()
    ));

  flusspferd::object history_file_(flusspferd::create_object());
  spec.set_property("history-file", history_file_);
  history_file_.set_property("argument", "required");
  flusspferd::create_native_function(history_file_, "callback",
    boost::function<void (flusspferd::value, std::string)>(
      phoenix::ref(history_file) = args::arg2
    ));

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

bool flusspferd_repl::getline(std::string &source, const char* prompt) {
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
