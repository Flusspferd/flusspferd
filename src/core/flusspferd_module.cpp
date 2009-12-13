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

#include "flusspferd/version.hpp"
#include "flusspferd/load_core.hpp"
#include "flusspferd/io/filesystem-base.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <stdlib.h>

using boost::optional;
static optional<std::string> get_exe_name();
static std::string get_exe_name_from_argv(std::string const &argv0);

using namespace flusspferd;
namespace fs = boost::filesystem;

void flusspferd::load_flusspferd_module(object container, std::string const &argv0) {
  object exports = container.get_property_object("exports");

  exports.define_property(
    "version",
    value(flusspferd::version()),
    read_only_property | permanent_property);

  exports.define_property(
    "relocatable",
    value(is_relocatable()),
    read_only_property | permanent_property);

  // Warning: This might not be right if the binary has been relocated!
  exports.define_property(
    "installPrefix",
    value(INSTALL_PREFIX),
    read_only_property | permanent_property);

  optional<std::string> exe = get_exe_name();

  if (exe) {
    exports.define_property(
      "executableName",
      value(io::fs_base::canonicalize(*exe).string()),
      read_only_property | permanent_property);
  }
  else {
    // Fall back to processing argv[0]
    exports.define_property(
      "executableName",
      value(get_exe_name_from_argv(argv0)),
      read_only_property | permanent_property);
  }
}

bool flusspferd::is_relocatable() {
#ifdef FLUSSPFERD_RELOCATABLE
  return true;
#else
  return false;
#endif
}

std::string flusspferd::version() {
  return FLUSSPFERD_VERSION;
}

// The fallback mechanism if platform specific method doesn't exist or failed
// 1. see if the file exists - if so canonicalize it
// 2. failing that, search in the path for binary named argv0
// 3. set it to argv0. Probably not usable, but oh well
std::string get_exe_name_from_argv(std::string const &argv0) {
  fs::path name(argv0);

  // 1. See if the file exists
  // Relative path. Complete using the initial value of cwd
  if(!name.has_root_path()) {
    name = fs::initial_path<fs::path>() / name;
  }

  fs::file_status s = fs::status(name);
  if (fs::exists(s) && !fs::is_directory(s)) {
    return io::fs_base::canonicalize(name).string();
  }

  // 2. search on the path
  char* path = getenv("PATH");

  if (!path)
    return argv0;

  // Reset it incase we prepended the cwd to it
  name = argv0;
  #ifdef WIN32
    name.replace_extension(".exe");

    char const *path_delim = ";";
  #else
    char const *path_delim = ":";
  #endif

  std::vector<std::string>path_dirs;
  boost::split(path_dirs, path, boost::is_any_of(path_delim));

  BOOST_FOREACH(std::string &d, path_dirs) {
    fs::path candidate = fs::path(d) / name;
    fs::file_status s = fs::status(candidate);
    if (fs::exists(s) && !fs::is_directory(s)) {
      return io::fs_base::canonicalize(candidate).string();
    }
  }

  // 3. Failure case is return whatever is in argv[0]
  return argv0;
}

// Various platform specific ways to get the path to the flusspferd interpreter
// Only Linux and OSX have been tested. Others are roughly correct

#if defined(FLUSSPFERD_SELF_EXE__NSGetExecutablePath)
  // OSX Version

  #include <vector>
  #include <mach-o/dyld.h>

  optional<std::string> get_exe_name() {
    std::vector<char> buf;
    buf.resize(1024);
    unsigned size = buf.size();

    int ret = _NSGetExecutablePath(&buf[0], &size);
    if (ret == -1) {
      // Buffer wasn't large enough. size now contains the size needed
      buf.resize(size+1);
      ret = _NSGetExecutablePath(&buf[0], &size);
    }

    if (ret == 0)
      return std::string(buf.begin(), buf.begin() + size);
    else
      return boost::none;
  }

#elif defined(FLUSSPFERD_SELF_EXE_proc_self_exe) || \
      defined(FLUSSPFERD_SELF_EXE_proc_curproc_file)
  // BSD and Linux Versions are similar - just different symlink to read

  optional<std::string> get_exe_name() {
  #ifdef FLUSSPFERD_SELF_EXE_proc_self_exe
    char const *link_name = "/proc/self/exe";
  #else
    char const *link_name = "/proc/curproc/file";
  #endif

    // The procfs might not be mounted.
    if (boost::filesystem::is_symlink(link_name))
      // This will be canonicalized outside.
      return std::string(link_name);
    else
      return boost::none;
  }

#elif defined(FLUSSPFERD_SELF_EXE_sysctl_PROC_PPATHNAME)
  // FreeBSD version

  #include <sys/types.h>
  #include <sys/sysctl.h>
  #include <unistd.h>

  optional<std::string> get_exe_name() {
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;
    char buf[MAX_PATH];
    size_t cb = sizeof(buf);
    if (sysctl(mib, 4, &cb, NULL, 0) == 0)
      return std::string(buf);
    else
      return boost::none;
  }

#elif defined(FLUSSPFERD_SELF_EXE_GetModuleFilename)
  // Windows version

  #include <windows.h>
  optional<std::string> get_exe_name() {
    char buf[MAX_PATH];
    DWORD len = GetModuleFileName( NULL, buf, MAX_PATH);
    if (len > 0)
      return std::string(buf, len);
    else
      return boost::none;
  }

#elif defined(FLUSSPFERD_SELF_EXE_getexecname)
  // Solaris version

  optional<std::string> get_exe_name() {
    char const * name = getexecname();
    return name ? name : boost::none;
  }

#else

  optional<std::string> get_exe_name() {
    return boost::none;
  }

#endif
