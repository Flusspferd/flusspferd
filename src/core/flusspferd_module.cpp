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
#include "flusspferd/create.hpp"
#include "flusspferd/io/filesystem-base.hpp"
#include <boost/optional.hpp>

using boost::optional;
static optional<std::string> get_exe_name();

using namespace flusspferd;

void flusspferd::load_flusspferd_module(object container) {
  object exports = container.get_property_object("exports");

  exports.define_property(
    "version",
    value(flusspferd::version()),
    read_only_property | permanent_property);

#ifdef FLUSSPFERD_RELOCATABLE
  exports.define_property(
    "relocatable",
    value(true),
    read_only_property | permanent_property);

#else
  exports.define_property(
    "relocatable",
    value(false),
    read_only_property | permanent_property);

  exports.define_property(
    "installPrefix",
    value(INSTALL_PREFIX),
    read_only_property | permanent_property);

#endif

  optional<std::string> exe = get_exe_name();

  if (exe) {
    exports.define_property(
      "executableName",
      value(io::fs_base::canonicalize(*exe).string()),
      read_only_property | permanent_property);
  }
}

std::string flusspferd::version() {
  return FLUSSPFERD_VERSION;
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
      return link_name;
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
    if (GetModuleFileName( NULL, buf, MAX_PATH))
      return buf;
    else
      return boost::none;
  }

#elif defined(FLUSSPFERD_SELF_EXE_getexecname)
  // Solaris version

  #include <stdlib.h>
  optional<std::string> get_exe_name() {
    char const * name = getexecname();
    return name ? name : boost::none;
  }

#else

  optional<std::string> get_exe_name() {
    return boost::none;
  }

#endif
