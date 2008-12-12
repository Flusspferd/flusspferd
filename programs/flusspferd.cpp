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

#include "flusspferd/security.hpp"
#include "flusspferd/importer.hpp"
#include "flusspferd/blob.hpp"
#include "flusspferd/class.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/context.hpp"
#include "flusspferd/value_io.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/current_context_scope.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/properties_functions.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

#ifdef HAVE_EDITLINE
#include <editline/readline.h>
#include <editline/history.h>
#endif

class flusspferd_repl {
  bool extfile;
  std::string file;
  std::istream in;

  bool config_loaded;
  char const *config_file;

  flusspferd::context co;
  flusspferd::current_context_scope scope;
  flusspferd::security &security;

  int argc;
  char ** argv;

  bool parse_cmdline();
  void load_config();

  bool getline(std::string &source, const char* prompt = "> ");
public:
  flusspferd_repl(int argc, char** argv);

  int run();
};



flusspferd_repl::flusspferd_repl(int argc, char **argv)
  : extfile(false),
    file("typein"),
    in(std::cin.rdbuf()),
    config_loaded(false),
    // Default - can be changed by -c cmd line option
    config_file(INSTALL_PREFIX "/etc/flusspferd/jsrepl.js"),
    co(flusspferd::context::create()),
    scope(flusspferd::current_context_scope(co)),
    security(flusspferd::security::create(flusspferd::global())),
    argc(argc),
    argv(argv)
{
  flusspferd::load_class<flusspferd::blob>();
  flusspferd::load_class<flusspferd::importer>();

  flusspferd::load_properties_functions();

  flusspferd::gc();

}

int flusspferd_repl::run() {
  if (!parse_cmdline())
    return 1;

  // if extfile is true, we've run it already, and aren't going to interactive
  if (extfile)
    return 0;

  if (!config_loaded)
    load_config();

  std::string source;
  unsigned int line = 0;

  while (getline(source)) {
    try {
      flusspferd::value v = flusspferd::evaluate(source, file.c_str(), ++line);
      if (!v.is_void())
        std::cout << v << '\n';
    }
    catch(std::exception &e) {
      std::cerr << "ERROR: " << e.what() << '\n';
    }
    flusspferd::gc();
  }

  return 0;
}

void print_help(char const *argv0) {
  std::cerr << "usage: " << argv0 <<
    " -h -c <file> -f <file> -i\n\n"
    "\t-h         displays this message\n"
    "\t-c <file>  load config from file\n"
    "\t-f <file>  open file as input\n"
    "\t-i         enter interactive mode (after running a file)\n\n";
}

void flusspferd_repl::load_config() {
  co.execute(config_file);
  config_loaded = true;

  // Get the prelude and execute it too
  flusspferd::value prelude = co.global().get_property("prelude");
  
  if (!prelude.is_void_or_null()) {
    co.execute(prelude.to_string().c_str());
  }
}

bool flusspferd_repl::parse_cmdline() {
  if (argc <= 1)
    return true;

  // TODO: We probably want to build up list of files to run, then populate
  // arguments object at top level (in JS) then run each file in turn
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "-h") == 0 ||
        std::strcmp(argv[i], "--help") == 0)
    {
      print_help(argv[0]);
      return false;
    }
    else if (std::strcmp(argv[i], "-c") == 0 ||
             std::strcmp(argv[i], "--config") == 0)
    {
      ++i;
      if (i == argc) {
        print_help(argv[0]);
        std::cerr << "ERROR: expected filename after " << argv[i-1]
                  << " option\n";
        return false;
      }
      config_file = argv[i];
    }
    else if (std::strcmp(argv[i], "-i") == 0 ||
             std::strcmp(argv[i], "--interactive") == 0)
    {
      extfile = false;
    }
    else if (std::strcmp(argv[i], "-f") == 0 ||
             std::strcmp(argv[i], "--file") == 0)
    {
      ++i;
      if (i == argc) {
        print_help(argv[0]);
        std::cerr << "ERROR: expected filename after " << argv[i-1]
                  << " option\n";
        return false;
      }
      file = argv[i];
      if (!config_loaded)
        load_config();
      extfile = true;
      co.execute(file.c_str());
    }
  }

  return true;
}

bool flusspferd_repl::getline(std::string &source, const char* prompt) {
#ifdef HAVE_EDITLINE
  if (!extfile) {
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
    repl.run();
  }
  catch(std::exception &e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    return 1;
  }
}
