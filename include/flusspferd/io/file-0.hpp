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

#ifndef FLUSSPFERD_IO_FILE_CLASS_HPP
#define FLUSSPFERD_IO_FILE_CLASS_HPP

#include "../string.hpp"
#include "../object.hpp"
#include <boost/filesystem.hpp>

namespace flusspferd {

void load_file_0_module(object container);
namespace io {

namespace file0 {

  string canonical(string path);
  boost::filesystem::path canonicalize(boost::filesystem::path in);

  void move(string source, string target);
  void remove(string target);
  void touch(string path, object mtime);

  void make_directory(string target);
  void remove_directory(string target);

  string read_link(string link);

  string working_directory();
  string change_workingDirectory(string path);

  string owner(string path);
  void change_owner(string path, string owner);

  int permissions(string path);
  void change_ermissions(string path, int newPerms);

  void link(string source, string target);
  void hard_link(string source, string target);

  bool exists(string path);
  bool is_file(string path);
  bool is_directory(string path);
  bool is_link(string path);
  bool is_readable(string path);
  bool is_writeable(string path);

  // Return a double since 1gb file limit on size would suck
  double size(string file);
  value last_modified(string path);
  bool same(string source, string target);

  array list(string path);
  object iterate(string path);
}

}}

#endif

