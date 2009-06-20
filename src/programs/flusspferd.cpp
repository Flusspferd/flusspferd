// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
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
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <list>

#ifdef HAVE_EDITLINE
#include <editline/readline.h>
#ifdef HAVE_EDITLINE_HISTORY_H
#include <editline/history.h>
#endif
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
  bool machine_mode;
  std::istream in;

  bool config_loaded;
  char const *config_file;

  flusspferd::context co;
  flusspferd::current_context_scope scope;

  bool running;
  int exit_code;

  std::string history_file;

  int argc;
  char ** argv;

  enum Type { File, Expression, IncludePath, Module };
  
  // Returns list of files / expressions to execute
  std::list<std::pair<std::string, Type> > parse_cmdline();
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
  std::list<std::pair<std::string, Type> > files = parse_cmdline();

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

void print_help(char const *argv0) {
  std::cerr << "usage: " << argv0 <<
" [option] ... [file | -] [arg] ...\n\n"
"Options\n"
"    -h                       Displays this message.\n"
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
"    -0                       (Interactive) machine command mode (separator\n"
"                             '\\0').\n"
"\n"
"    -e <expr>\n"
"    --expression <expr>      Evaluate the expression.\n"
"\n"
"    -f <file>\n"
"    --file <file>            Run this file before standard script handling.\n"
"\n"
"    -I <path>\n              Add include path."
"\n"
"    -M <module>\n            Load module."
"\n"
"    --no-global-history\n    Do not use a global history in interactive mode\n"
"\n"
"    --history-file <file>\n  Sets history file (default: ~/.flusspferd-history)\n"
"\n"
"    --                       Stop processing options.\n\n";
}

void print_version() {
  std::cout << "flusspferd shell version: " << FLUSSPFERD_VERSION << '\n';
  std::cout << "flusspferd library version: " << flusspferd::version() << '\n';
  std::cout.flush();
}

void flusspferd_repl::load_config() {
  flusspferd::execute(config_file);
  config_loaded = true;

  // Get the prelude and execute it too
  flusspferd::value prelude = co.global().get_property("prelude");
  
  if (!prelude.is_undefined_or_null()) {
    flusspferd::execute(prelude.to_string().c_str(), flusspferd::global().prototype());
  }
}

std::list<std::pair<std::string, flusspferd_repl::Type> >
flusspferd_repl::parse_cmdline() {
  bool interactive_set = false;

  std::list<std::pair<std::string, Type> > files;

  int i=1;

  for (i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (std::strcmp(argv[i], "--") == 0)
      {
        i++;
        break;
      }
      else if (std::strcmp(argv[i], "-h") == 0 ||
          std::strcmp(argv[i], "--help") == 0)
      {
        print_help(argv[0]);
        throw flusspferd::js_quit();
      }
      else if (std::strcmp(argv[i], "-v") == 0 ||
          std::strcmp(argv[i], "--version") == 0)
      {
        print_version();
        throw flusspferd::js_quit();
      }
      else if (std::strcmp(argv[i], "-c") == 0 ||
               std::strcmp(argv[i], "--config") == 0)
      {
        ++i;
        if (i == argc) {
          print_help(argv[0]);
          std::string msg = "expected filename after ";
          msg += argv[i-1];
          msg += " option\n";
          throw std::runtime_error(msg);
        }
        config_file = argv[i];
      }
      else if (std::strcmp(argv[i], "-i") == 0 ||
               std::strcmp(argv[i], "--interactive") == 0)
      {
        interactive = true;
        interactive_set = true;
      }
      else if (std::strcmp(argv[i], "-0") == 0)
      {
        interactive = true;
        interactive_set = true;
        machine_mode = true;
      }
      else if (std::strcmp(argv[i], "-f") == 0 ||
               std::strcmp(argv[i], "--file") == 0)
      {
        ++i;
        if (i == argc) {
          print_help(argv[0]);
          std::string msg = "expected filename after ";
          msg += argv[i-1];
          msg += " option\n";
          throw std::runtime_error(msg);
        }
        std::string file = argv[i];
        if (!interactive_set)
          interactive = false;
        files.push_back(std::make_pair(file, File));
      }
      else if (std::strcmp(argv[i], "-e") == 0 ||
               std::strcmp(argv[i], "--expression") == 0)
      {
        ++i;
        if (i == argc) {
          print_help(argv[0]);
          std::string msg = "expected expression after ";
          msg += argv[i-1];
          msg += " option\n";
          throw std::runtime_error(msg);
        }
        std::string file = argv[i];
        if (!interactive_set)
          interactive = false;
        files.push_back(std::make_pair(file, Expression));
      }
      else if (std::strcmp(argv[i], "-I") == 0)
      {
        ++i;
        if (i == argc) {
          print_help(argv[0]);
          std::string msg = "expected expression after ";
          msg += argv[i-1];
          msg += " option\n";
          throw std::runtime_error(msg);
        }
        std::string path = argv[i];
        files.push_back(std::make_pair(path, IncludePath));
      }
      else if (std::strcmp(argv[i], "-M") == 0)
      {
        ++i;
        if (i == argc) {
          print_help(argv[0]);
          std::string msg = "expected expression after ";
          msg += argv[i-1];
          msg += " option\n";
          throw std::runtime_error(msg);
        }
        std::string path = argv[i];
        files.push_back(std::make_pair(path, Module));
      }
      else if (std::strcmp(argv[i], "--no-global-history") == 0)
      {
        history_file.clear();
      }
      else if (std::strcmp(argv[i], "--history-file") == 0)
      {
        ++i;
        if (i == argc) {
          print_help(argv[0]);
          std::string msg = "expected expression after ";
          msg += argv[i-1];
          msg += " option\n";
          throw std::runtime_error(msg);
        }
        history_file = argv[i];
      }
      else
      {
        print_help(argv[0]);
        throw std::runtime_error(std::string("invalid option: ")+argv[i]+"\n");
      }
    }
    else
      break; // Not an option, stop looking for one
  }

  flusspferd::array args =
    flusspferd::global()
      .call("require", "system").to_object()
      .get_property_object("args");

  args.set_element(0, flusspferd::value(""));

  if (i < argc) {
    // some cmd line args left.
    // first one is file
    // others go into arguments array
    std::string file = argv[i++];
    if (file == "-") {
      // TODO: Maybe check if stdin is actualy connected to a terminal?
      interactive = true;
    }
    else {
      files.push_back(std::make_pair(file, File));
    }

    args.set_element(0, flusspferd::value(file));

    int x = 1;
    for (; i < argc; ++i) {
      args.set_element(
        x++,
        flusspferd::value(const_cast<char const*>(argv[i])));
    }
  }

  return files;
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
    if (linep[0] != '\0')
        add_history(linep);
    source = linep;
    source += '\n';
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
