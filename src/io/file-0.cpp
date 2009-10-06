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

using namespace flusspferd;
namespace file0 =  flusspferd::io::file0;
namespace fs = boost::filesystem;

void flusspferd::load_file_0_module(object container) {
  object exports = container.get_property_object("exports");

  create_native_function(exports, "canonical", &file0::canonical);
  create_native_function(exports, "lastModified", &file0::last_modified);
}

fs::path canonicalize(fs::path in);

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
