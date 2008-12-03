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

#include "flusspferd/xml/xml.hpp"
#include "flusspferd/io/io.hpp"
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

#ifdef HAVE_READLINE
#  include <readline/readline.h>
#  include <readline/history.h>
#endif

bool extfile = false;
std::string file = __FILE__;
std::istream in(std::cin.rdbuf());

void print_help(char const *argv0) {
  std::cerr << "usage: " << argv0 <<
    " -h -f <file>\n\n"
    "\t-h         displays this message\n"
    "\t-f <file>  open file as input\n\n";
}

bool parse_cmd(int argc, char **argv) {
  if (argc <= 1)
    return true;

  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "-h") == 0 ||
        std::strcmp(argv[i], "--help"))
    {
      print_help(argv[0]);
      return false;
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
      std::ifstream fin(file.c_str());
      in.rdbuf(fin.rdbuf());
      extfile = true;
    }
  }

  return true;
}

bool getline(std::string &source, const char* prompt = "> ") {
#ifdef HAVE_READLINE
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
  if(!parse_cmd(argc, argv))
    return 1;

  try {
    flusspferd::init::initialize();
    flusspferd::context co = flusspferd::context::create();
    flusspferd::current_context_scope scope(co);

    #ifdef FLUSSPFERD_HAVE_XML
    flusspferd::xml::load_xml();
    #endif

    flusspferd::load_class<flusspferd::blob>();
    flusspferd::load_class<flusspferd::importer>();

    flusspferd::importer::add_preloaded("IO", &flusspferd::io::load_io);

    flusspferd::load_properties_functions();

    co.execute("prelude.js");

    flusspferd::gc();

    std::string source;
    unsigned int line = 0;

    while (getline(source)) {
      try {
        flusspferd::value v =
          flusspferd::evaluate(source, file.c_str(), ++line);
        if (!v.is_void())
          std::cout << v << '\n';
      }
      catch(std::exception &e) {
        std::cerr << "ERROR: " << e.what() << '\n';
      }
      flusspferd::gc();
    }
  }
  catch(std::exception &e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    return 1;
  }
}
