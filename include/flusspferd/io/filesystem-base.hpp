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

#ifndef FLUSSPFERD_IO_FILESYSTEM_BASE_HPP
#define FLUSSPFERD_IO_FILESYSTEM_BASE_HPP

#include "../string.hpp"
#include "../object.hpp"
#include <boost/filesystem.hpp>

namespace flusspferd {

void load_filesystem_base_module(object container);

namespace io {

namespace fs_base {

  object raw_open(char const* name, value mode, value permissions);

  string canonical(std::string const &path);
  boost::filesystem::path canonicalize(boost::filesystem::path in);

  void move(std::string const &source, std::string const &target);
  void remove(std::string const &target);
  void touch(std::string const &path, object mtime);

  void make_directory(std::string const &target);
  void remove_directory(std::string const &target);

  string read_link(std::string const &link);

  string working_directory();
  void change_working_directory(std::string const &path);

  string owner(std::string const &path);
  void change_owner(std::string const &path, std::string owner);

  void link(std::string const &source, std::string const &target);
  void hard_link(std::string const &source, std::string const &target);

  bool exists(std::string const &path);
  bool is_file(std::string const &path);
  bool is_directory(std::string const &path);
  bool is_link(std::string const &path);
  bool is_readable(std::string const &path);
  bool is_writeable(std::string const &path);

  // Return a double since 1gb file limit on size would suck
  double size(std::string const &file);
  value last_modified(std::string const &path);
  bool same(std::string const &source, std::string const &target);

  array list(std::string const &path);
  object iterate(std::string const &path);
}

}}

#endif

