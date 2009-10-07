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

#include "flusspferd/io/file-0.hpp"
#include "flusspferd.hpp"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>

#ifdef WIN32
#include <stdio.h>
#else
#include <unistd.h>
#endif

using namespace flusspferd;
namespace file0 =  flusspferd::io::file0;
namespace fs = boost::filesystem;

void flusspferd::load_file_0_module(object container) {
  object exports = container.get_property_object("exports");

  create_native_function(exports, "canonical", &file0::canonical);
  create_native_function(exports, "lastModified", &file0::last_modified);
  create_native_function(exports, "touch", &file0::touch);
  create_native_function(exports, "size", &file0::size);


  create_native_function(exports, "exists", &file0::exists);
  create_native_function(exports, "isFile", &file0::is_file);
  create_native_function(exports, "isDirectory", &file0::is_directory);
  create_native_function(exports, "isLink", &file0::is_link);
  create_native_function(exports, "isReadable", &file0::is_readable);
  create_native_function(exports, "isWriteable", &file0::is_writeable);


  create_native_function(exports, "link", &file0::link);
  create_native_function(exports, "hardLink", &file0::hard_link);
  create_native_function(exports, "readLink", &file0::read_link);
}

string file0::canonical(string path) {
  return canonicalize(path.to_string()).string();
}

// Resolve symlinks
fs::path file0::canonicalize(fs::path in) {
  fs::path accum;
  char buff[PATH_MAX];

  if (!in.has_root_path()) {
    // dir is relative!
    accum = fs::system_complete(".");
    if (*--accum.end() == ".")
      accum.remove_filename();
  }

  BOOST_FOREACH(fs::path seg, in) {
    if (seg == ".")
      continue;

    // We've already canon'd the path's parent, so just remove the last dir
    if (seg == "..") {
      accum = accum.remove_filename();
      continue;
    }

    accum /= seg;
    if (fs::is_symlink(accum)) {
      ssize_t len = readlink(accum.string().c_str(), buff, PATH_MAX);
      if (len == -1) {
        //TODO: How do i use boost to get a nicer errmessage? Also should actually use errno
        throw std::string("Path too long");
      }
      fs::path link_path = std::string(buff, len);

      // An absolute link
      if (link_path.has_root_path())
        accum = canonicalize(link_path);
      else {
        accum.remove_filename();
        accum = canonicalize(accum / link_path);
      }
    }
  }

  // This trickery forces a trailing / onto the dir
  accum /= ".";
  accum.remove_filename();

  return accum;
}

value file0::last_modified(string path) {
  std::time_t last_mod = fs::last_write_time(path.to_string());

  // TODO: Is there any way that isn't so truely horrible?
  std::string js = "new Date(";
  return evaluate(js + boost::lexical_cast<std::string>(last_mod*1000.0) + ")");
}

void file0::touch(string str, object mtime_o) {
  object date = global().get_property_object("Date");
  value ctor;
  if (mtime_o.is_null() ||
     !(ctor = mtime_o.get_property("constructor")).is_object() ||
     ctor.get_object() != date)
  {
    throw exception("touch expects a Date as it's second argument", "TypeError");
  }

  double msecs = mtime_o.call("valueOf").to_number();
  std::time_t mtime = msecs/1000;

  fs::path p(str.to_string());
  if (!fs::exists(p)) {
    // File doesn't exist, create
    fs::ofstream f(p);
  }

  fs::last_write_time(p, mtime);
}

// JS has no concept of unit, and double has a 53 bit mantissa, which means we
// can store up to 9*10^E15 (2^53, 8192TB ) without loosing precisions. Much
// better than only 30bits == 1gb! eek
double file0::size(string file) {
  uintmax_t fsize = fs::file_size(file.to_string());
  return fsize;
}

bool file0::exists(string p) {
  return fs::exists(p.to_string());
}

bool file0::is_file(string p) {
  return fs::is_regular_file(p.to_string());
}

bool file0::is_directory(string p) {
  return fs::is_directory(p.to_string());
}

bool file0::is_link(string p) {
  return fs::is_symlink(p.to_string());
}

bool file0::is_readable(string p) {
  return access(p.to_string().c_str(), R_OK) != -1;
}

bool file0::is_writeable(string str) {
  fs::path p(str.to_string());

  if (access(p.string().c_str(), W_OK) != -1)
    return true;

  // Might be false because it doesn't exist, in which case check we can write
  // to the dir its in
  if (!fs::exists(p)) {
    p.remove_filename();
    p = canonicalize(p);
    return access(p.string().c_str(), W_OK) != -1;
  }
  return false;
}

void file0::link(string source, string target) {
  if (symlink(source.to_string().c_str(), target.to_string().c_str()) == 0)
    return;

  // TODO: paths and system error message!
  throw exception("Error creating symbolic link");
}

void file0::hard_link(string source, string target) {
  if (::link(source.to_string().c_str(), target.to_string().c_str()) == 0)
    return;

  // TODO: paths and system error message!
  throw exception("Error creating hard link");
}

string file0::read_link(string link) {
  std::string s = link.to_string();
  if (!fs::is_symlink(s)) {
    throw exception("Cannot readLink: " + s + " is not a link");
  }

  char buff[PATH_MAX];

  ssize_t len = readlink(s.c_str(), buff, PATH_MAX);
  if (len == -1) {
    //TODO: How do i use boost to get a nicer errmessage? Also should actually use errno
    throw std::string("Path too long");
  }
  return string(buff, len);
}
