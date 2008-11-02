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

#include "flusspferd/file_class.hpp"
#include "flusspferd/class.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/context.hpp"
#include "flusspferd/value_io.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/current_context_scope.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

//#include <js/jsapi.h> // DEBUG

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
  if(argc > 1) {
    for(int i = 1; i < argc; ++i) {
      if(std::strcmp(argv[i], "-h") == 0 ||
         std::strcmp(argv[i], "--help"))
      {
        print_help(argv[0]);
        return false;
      }
      else if(std::strcmp(argv[i], "-f") == 0 ||
              std::strcmp(argv[i], "--file") == 0)
      {
        ++i;
        if(i == argc) {
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
  }
  return true;
}

int main(int argc, char **argv) {
  if(!parse_cmd(argc, argv))
    return 1;

  try {
    flusspferd::init::initialize();
    flusspferd::context co = flusspferd::context::create();
    flusspferd::current_context_scope scope(co);

    flusspferd::load_class<flusspferd::file_class>();

    co.gc();

    std::string source;
    unsigned int line = 0;
    if(!extfile)
      std::cout << "> ";
    while(std::getline(in, source)) {
      try {
        flusspferd::value v = flusspferd::evaluate(source, file.c_str(), ++line);
        if(!v.is_void() && !v.is_null())
          std::cout << v << '\n';
      }
      catch(std::exception &e) {
        std::cerr << "ERROR: " << e.what() << '\n';
      }
      co.gc();
      if(!extfile)
        std::cout << "> ";
    }
  }
  catch(std::exception &e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    return 1;
  }
}
