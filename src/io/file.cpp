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
#include <windows.h>

#define creat _creat

namespace {
  std::string compose_error_message(char const *what, char const *filename = 0x0) {
    char *message;
    DWORD const error = GetLastError();

    std::string ret = what;
    DWORD len = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        0x0, error, 0, (char*)&message,
        0, 0x0);

    if (len) {
      // Strip trailing new lines
      while (message[len-1] == '\n' || message[len-1] == '\r') len--;
      ret += ": '" + std::string(message,len) + "'";
      LocalFree(message);
    }
    if (filename) {
      ret += std::string(" (") + filename + ')';
    }
    return ret;
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


file::file(object const &obj, char const* name, value mode)
  : base_type(obj, (std::streambuf*)0), p(new impl)
{
  set_streambuf(p->stream.rdbuf());
  open(name, mode);
}

file::~file()
{}

void file::open(char const *name, value options) {
  security &sec = security::get();

  if (boost::filesystem::is_directory(std::string(name))) {
    throw exception(
      std::string("Could not open file: it is a directory (")+ name + ")"
    );
  }

  std::ios::openmode open_mode = std::ios::openmode();

  bool exclusive = false, create = false;

  if (options.is_string()) {
    // String modes always set create

    std::string mode = options.to_std_string();
    if (mode == "r")
      open_mode = std::ios::in;
    else if (mode == "r+")
      open_mode = std::ios::in | std::ios::out;
    else if (mode == "r+x") {
      open_mode = std::ios::in | std::ios::out;
      exclusive = create = true;
    }
    else if (mode == "w") {
      open_mode = std::ios::out;
      create = true;
    }
    else if (mode == "wx") {
      open_mode = std::ios::out;
      exclusive = create = true;
    }
    else if (mode == "w+x") {
      open_mode = std::ios::out | std::ios::in | std::ios::trunc;
      exclusive = create = true;
    }
    else {
      throw exception(str(format("File.open: mode '%s' not supported (yet?)") % mode));
    }
  }else if (options.is_object()) {
    object obj = options.get_object();

    create = obj.get_property("create").to_boolean();

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
      if (!create)
        throw exception("File.open: exclusive mode can only be used with create");
      exclusive = create = true;
    }

  }else if (options.is_undefined_or_null()) {
    open_mode = std::ios::in | std::ios::out;
  }else {
    throw exception("File.open: Invalid options argument", "TypeError");
  }

  unsigned sec_mode = 0;

  if (open_mode & std::ios::in)  sec_mode |= security::READ;
  if (open_mode & std::ios::out) sec_mode |= security::WRITE;
  if (create)                    sec_mode |= security::CREATE;

  if (!sec.check_path(name, sec_mode)) {
    throw exception(str(
      format("File.open: could not open file: 'denied by security' (%s)") % name
    ));
  }

  if (create) {
    // C++ streams don't support O_EXCL|O_CREAT modes. Fall back to open
    unsigned o_mode = exclusive
                    ? O_CREAT|O_EXCL
                    : O_CREAT;
    int fd = ::open(name, o_mode, 0666);
    if (fd == -1)
      throw exception(compose_error_message("File.open: couldn't create file", name));

    // Done  - got the file (exclusively) created.
    ::close(fd);
  }

  p->stream.open(name, open_mode);

  if (!p->stream)
    throw exception(compose_error_message("Could not open file", name));

  define_property("fileName", string(name),
                  permanent_property | read_only_property );
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
