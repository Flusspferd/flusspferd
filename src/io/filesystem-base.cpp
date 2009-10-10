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

#include "flusspferd/io/filesystem-base.hpp"
#include "flusspferd/io/file.hpp"
#include "flusspferd.hpp"
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>

#ifdef WIN32
#include <stdio.h>
#else
#include <unistd.h>
#endif

using namespace flusspferd;
using boost::format;
namespace fs_base =  flusspferd::io::fs_base;
namespace fs = boost::filesystem;

// owner: No such file or directory "foo/bar"
const format error_fmt("%1%: %2% \"%3%\"");
// symlink: No such file or directory "foo/bar", "baz"
const format error_fmt2("%1%: %2% \"%3%\", \"%4%\"");
// lastModified: access to "foo/bar" deined by security
const format error_sec("%1%: security deined operation on \"%2%\"");

void flusspferd::load_filesystem_base_module(object container) {
  object exports = container.get_property_object("exports");

  create_native_function(exports, "rawOpen", &fs_base::raw_open);

  create_native_function(exports, "canonical", &fs_base::canonical);
  create_native_function(exports, "lastModified", &fs_base::last_modified);
  create_native_function(exports, "touch", &fs_base::touch);
  create_native_function(exports, "size", &fs_base::size);


  create_native_function(exports, "exists", &fs_base::exists);
  create_native_function(exports, "isFile", &fs_base::is_file);
  create_native_function(exports, "isDirectory", &fs_base::is_directory);
  create_native_function(exports, "isLink", &fs_base::is_link);
  create_native_function(exports, "isReadable", &fs_base::is_readable);
  create_native_function(exports, "isWriteable", &fs_base::is_writeable);
  create_native_function(exports, "same", &fs_base::same);


  create_native_function(exports, "link", &fs_base::link);
  create_native_function(exports, "hardLink", &fs_base::hard_link);
  create_native_function(exports, "readLink", &fs_base::read_link);


  create_native_function(exports, "makeDirectory", &fs_base::make_directory);
  create_native_function(exports, "removeDirectory", &fs_base::remove_directory);



  create_native_function(exports, "move", &fs_base::move);
  create_native_function(exports, "remove", &fs_base::remove);



  create_native_function(exports, "workingDirectory", &fs_base::working_directory);
  create_native_function(exports, "changeWorkingDirectory", &fs_base::change_working_directory);



  create_native_function(exports, "list", &fs_base::list);

#ifdef FLUSSPFERD_HAVE_POSIX
  create_native_function(exports, "owner", &fs_base::owner);
#endif
}

object fs_base::raw_open(char const* name, value mode, value perms) {
  // TODO: Deal with permissions somewhere :)
  if (!perms.is_undefined_or_null())
    throw exception("rawOpen: permissions not yet supported");

  return create_native_object<io::file>(object(), name, mode);
}

string fs_base::canonical(std::string const &path) {
  if (!security::get().check_path(path, security::ACCESS)) {
    throw exception(str(format(error_sec) % "canonical" % path));
  }

  return canonicalize(path).string();
}

// Resolve symlinks
fs::path fs_base::canonicalize(fs::path in) {
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
        format fmt = format(error_fmt) % "canonical"
                                       % std::strerror(errno)
                                       % accum;
        throw exception(fmt.str());
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

value fs_base::last_modified(std::string const &path) {
  if (!security::get().check_path(path, security::ACCESS)) {
    throw exception(str(format(error_sec) % "lastModified" % path));
  }

  std::time_t last_mod = fs::last_write_time(path);

  // TODO: Is there any way that isn't so truely horrible?
  std::string js = "new Date(";
  return evaluate(js + boost::lexical_cast<std::string>(last_mod*1000.0) + ")");
}

void fs_base::touch(std::string const &str, object mtime_o) {
  object date = global().get_property_object("Date");
  value ctor;
  std::size_t mtime;

  if (!mtime_o.is_null()) {
    if (!(ctor = mtime_o.get_property("constructor")).is_object() ||
        ctor.get_object() != date)
    {
      throw exception("touch: expects a Date as it's second argument if present",
                      "TypeError");
    }

    double msecs = mtime_o.call("valueOf").to_number();
    mtime = msecs/1000;
  }
  else {
    mtime = time(NULL);
  }

  security &sec = security::get();
  fs::path p(str);
  if (!fs::exists(p)) {
    if (!sec.check_path(str, security::CREATE))
      throw exception(boost::str(format(error_sec) % "touch" % str));
    // File doesn't exist, create
    fs::ofstream f(p);
  }

  if (!sec.check_path(str, security::WRITE))
    throw exception(boost::str(format(error_sec) % "touch" % str));
  fs::last_write_time(p, mtime);
}

// JS has no concept of unit, and double has a 53 bit mantissa, which means we
// can store up to 9*10^E15 (2^53, 8192TB ) without loosing precisions. Much
// better than only 30bits == 1gb! eek
double fs_base::size(std::string const &file) {
  if (!security::get().check_path(file, security::ACCESS)) {
    throw exception(str(format(error_sec) % "size" % file));
  }
  uintmax_t fsize = fs::file_size(file);
  return fsize;
}

bool fs_base::exists(std::string const &p) {
  if (!security::get().check_path(p, security::ACCESS)) {
    throw exception(str(format(error_sec) % "exists" % p));
  }
  return fs::exists(p);
}

bool fs_base::is_file(std::string const &p) {
  if (!security::get().check_path(p, security::ACCESS)) {
    throw exception(str(format(error_sec) % "isFile" % p));
  }
  return fs::is_regular_file(p);
}

bool fs_base::is_directory(std::string const &p) {
  if (!security::get().check_path(p, security::ACCESS)) {
    throw exception(str(format(error_sec) % "isDirectory" % p));
  }
  return fs::is_directory(p);
}

bool fs_base::is_link(std::string const &p) {
  if (!security::get().check_path(p, security::ACCESS)) {
    throw exception(str(format(error_sec) % "isLink" % p));
  }
  return fs::is_symlink(p);
}

bool fs_base::is_readable(std::string const &p) {
  security &sec = security::get();
  if (!sec.check_path(p, security::ACCESS)) {
    throw exception(str(format(error_sec) % "isReadable" % p));
  }

  return (!security::get().check_path(p, security::READ)) &&
         access(p.c_str(), R_OK) != -1;
}

bool fs_base::is_writeable(std::string const &str) {
  fs::path p(str);
  security &sec = security::get();
  if (!sec.check_path(str, security::ACCESS)) {
    throw exception(boost::str(format(error_sec) % "isWriteable" % str));
  }

  if (!sec.check_path(str, security::WRITE)) {
    // Denied by security - never writable
    return false;
  }

  if (access(str.c_str(), W_OK) != -1)
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

bool fs_base::same(std::string const &source, std::string const &target) {\
  security &sec = security::get();
  if (!sec.check_path(source, security::ACCESS)) {
    throw exception(boost::str(format(error_sec) % "same" % source));
  }
  if (!sec.check_path(target, security::ACCESS)) {
    throw exception(boost::str(format(error_sec) % "same" % target));
  }
  // TODO: test if this behaves right w.r.t. symlinks
  return fs::equivalent(source, target);
}

void fs_base::link(std::string const &source, std::string const &target) {
  security &sec = security::get();
  if (!sec.check_path(source, security::ACCESS)) {
    throw exception(boost::str(format(error_sec) % "link" % source));
  }
  if (!sec.check_path(target, security::WRITE|security::CREATE)) {
    throw exception(boost::str(format(error_sec) % "link" % target));
  }

  if (symlink(source.c_str(), target.c_str()) == 0)
    return;

  // TODO: paths and system error message!
  format e = format(error_fmt2)
           % "link"
           % std::strerror(errno)
           % source
           % target;
  throw exception(e.str());
}

void fs_base::hard_link(std::string const &source, std::string const &target) {
  security &sec = security::get();
  if (!sec.check_path(source, security::ACCESS)) {
    throw exception(boost::str(format(error_sec) % "hardLink" % source));
  }
  if (!sec.check_path(target, security::WRITE|security::CREATE)) {
    throw exception(boost::str(format(error_sec) % "hardLink" % target));
  }

  if (::link(source.c_str(), target.c_str()) == 0)
    return;

  // TODO: paths and system error message!
  format e = format(error_fmt2)
           % "hardLink"
           % std::strerror(errno)
           % source
           % target;
  throw exception(e.str());
}

string fs_base::read_link(std::string const &link) {
  if (!security::get().check_path(link, security::READ)) {
    throw exception(boost::str(format(error_sec) % "readLink" % link));
  }

  char buff[PATH_MAX];

  ssize_t len = readlink(link.c_str(), buff, PATH_MAX);
  if (len == -1) {
    format e = format(error_fmt)
             % "readLink"
             % std::strerror(errno)
             % link;
    throw exception(e.str());
  }
  return string(buff, len);
}


void fs_base::make_directory(std::string const &dir) {
  if (!security::get().check_path(dir, security::CREATE)) {
    throw exception(boost::str(format(error_sec) % "makeDirectory" % dir));
  }

  fs::create_directory(dir);
}

void fs_base::remove_directory(std::string const &dir) {
  fs::path p(dir);

  if (!fs::exists(p))
    throw exception("removeDirectory: " + p.string() + " doesn't exist");
  if (!fs::is_directory(p))
    throw exception("removeDirectory: " + p.string() + " isn't a directory");

  if (!security::get().check_path(dir, security::ACCESS|security::WRITE)) {
    throw exception(boost::str(format(error_sec) % "removeDirectory" % dir));
  }

  fs::remove(p);
}

void fs_base::move(std::string const &source, std::string const &target) {
  security &sec = security::get();
  if (!sec.check_path(source, security::READ)) {
    throw exception(boost::str(format(error_sec) % "move" % source));
  }
  if (!sec.check_path(target, security::WRITE|security::CREATE)) {
    throw exception(boost::str(format(error_sec) % "move" % target));
  }

  fs::rename(source, target);
}

void fs_base::remove(std::string const &path) {
  if (!security::get().check_path(path, security::WRITE)) {
    throw exception(boost::str(format(error_sec) % "remove" % path));
  }

  fs::path p(path);

  if (!fs::exists(p))
    throw exception("remove: " + p.string() + " doesn't exist");
  // Remove doesn't operate on dirs, just (plain,symlink,special) files
  if (fs::is_directory(p))
    throw exception("remove: " + p.string() + " isn't a file");
  fs::remove(p);
}

string fs_base::working_directory() {
  //return fs::current_path().string();
  return fs::current_path<fs::path>().string();
}

void fs_base::change_working_directory(std::string const &str) {
  if (!security::get().check_path(str, security::ACCESS)) {
    throw exception(boost::str(format(error_sec) % "changeWorkingDirectory" % str));
  }

  fs::current_path(str);
}

array fs_base::list(std::string const &dir) {
  if (!security::get().check_path(dir, security::ACCESS)) {
    throw exception(boost::str(format(error_sec) % "list" % dir));
  }


  root_array ret(create_array());

  fs::basic_directory_iterator<fs::path> it(dir);

  for (;  it != fs::directory_iterator(); ++it) {
    ret.call("push", it->path().string());
  }

  return ret;
}

#ifdef FLUSSPFERD_HAVE_POSIX
#include <sys/stat.h>
#include <errno.h>
#include <cstring>
#include <pwd.h>

// stat a file, and throw a nicer error message on failure
static void _get_stat(std::string const &path, struct stat *buf,
    char const *fn_name)
{
  int ret = ::stat(path.c_str(), buf);
  if (ret != 0) {
    format fmt = format(error_fmt) % fn_name % std::strerror(errno) % path;
    throw exception(fmt.str());
  }
}

string fs_base::owner(std::string const &path) {
  if (!security::get().check_path(path, security::ACCESS)) {
    throw exception(boost::str(format(error_sec) % "owner" % path));
  }

  struct stat buf;
  _get_stat(path, &buf, "owner");

  struct passwd *p = getpwuid(buf.st_uid);

  if (p)
    return p->pw_name;
  else
    return boost::lexical_cast<std::string>(buf.st_uid);
}

#endif
