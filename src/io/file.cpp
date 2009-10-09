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

#include "flusspferd/io/file.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/string_io.hpp"
#include "flusspferd/create.hpp"
#include <boost/scoped_array.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef WIN32
#include <io.h>

#define creat _creat

namespace {
  std::string compose_error_message(std::string const &what, char const *filename = 0x0) {
#if 0
    /* yuck Windoze API.
       Is TCHAR the way to go, since we only want char? (no wchar_t stuff?)
     */
    TCHAR *message;
    DWORD const error = GetLastError();

    if(FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        0x0, error, 0, message,
        0, 0x0) == 0)
    {
      return what;
    }
    else
    {
      std::string ret = what + ": '" + message + "'";
      LocalFree(message);
      return ret;
    }
#else
    return what + filename;
#endif
  }
}
#else
#include <errno.h>
#include <cstring>
namespace {
  std::string compose_error_message(char const *what, char const *filename = 0x0) {
    std::string error = std::string(what) + ": '" + std::strerror(errno) + "'";
    if (filename) {
      error += std::string(" (") + filename + ')';
    }
    return error;
  }
}
#endif

using namespace flusspferd;
using namespace flusspferd::io;
using namespace boost;

class file::impl {
public:
  std::fstream stream;
};

file::file(object const &obj, call_context &x)
  : base_type(obj, (std::streambuf*)0), p(new impl)
{
  set_streambuf(p->stream.rdbuf());
  if (!x.arg.empty()) {
    call("open", x.arg);
  }
}

file::~file()
{}

void file::open(char const *name, value options) {
  security &sec = security::get();

  if (!sec.check_path(name, security::READ_WRITE))
    throw exception("Could not open file (security)");

  std::ios::openmode open_mode = std::ios::openmode();

  bool exclusive_create = false;

  // TODO: Support more open modes, check defaults
  if (options.is_string()) {
    std::string mode = options.to_std_string();
    if (mode == "r")
      open_mode = std::ios::in;
    else if (mode == "r+")
      open_mode = std::ios::in | std::ios::out;
    else if (mode == "r+x") {
      open_mode = std::ios::in | std::ios::out;
      exclusive_create = true;
    }
    else if (mode == "w")
      open_mode = std::ios::out;
    else if (mode == "wx") {
      open_mode = std::ios::out;
      exclusive_create = true;
    }
    else if (mode == "w+x") {
      open_mode = std::ios::out | std::ios::in | std::ios::trunc;
      exclusive_create = true;
    }
    else {
      throw exception(str(format("File.open: mode '%s' not supported (yet?)") % mode));
    }
  }else if (options.is_object()) {
    object obj = options.get_object();

    if (obj.get_property("read").to_boolean())
      open_mode |= std::ios::in;
    if (obj.get_property("write").to_boolean())
      open_mode |= std::ios::out;
    if (obj.get_property("truncate").to_boolean())
      open_mode |= std::ios::trunc;
    if (obj.get_property("append").to_boolean()) {
      if (!(open_mode & std::ios::out)) {
        throw exception("File.open: append mode can only be used with write");
      }
      open_mode |= std::ios::app | std::ios::out;
    }
    if (obj.get_property("exclusive").to_boolean()) {
      bool create = obj.get_property("create").to_boolean();
      if (!create)
        throw exception("File.open: exclusive mode can only be used with create");
      exclusive_create = true;
    }
    // TODO: Work out how to handle create: false
  }else if (options.is_undefined_or_null()) {
    open_mode = std::ios::in | std::ios::out;
  }else {
    throw exception("File.open: Invalid options argument", "TypeError");
  }

  if (exclusive_create) {
    // C++ streams don't support O_EXCL|O_CREAT mode. Fall back to open
    int fd = ::open(name, O_CREAT|O_EXCL);
    if (fd == -1)
      throw exception(compose_error_message("File.open: couldn't create exclusive file", name));

    // Done  - got the file exclusively created.
    ::close(fd);
  }

  p->stream.open(name, open_mode);

  define_property("fileName", string(name),
                  permanent_property | read_only_property );

  if (!p->stream)
    throw exception(compose_error_message("Could not open file", name));
}

void file::close() {
  p->stream.close();
  delete_property("fileName");
}

void file::create(char const *name, boost::optional<int> mode) {
  security &sec = security::get();

  if (!sec.check_path(name, security::CREATE))
    throw exception("Could not create file (security)");

  if (creat(name, mode.get_value_or(0666)) < 0)
    throw exception(compose_error_message("Could not create file", name));
}

bool file::exists(char const *name) {
  security &sec = security::get();

  if (!sec.check_path(name, security::ACCESS))
    throw exception("Could not check whether file exists (security)");

  return boost::filesystem::exists(name);
}
