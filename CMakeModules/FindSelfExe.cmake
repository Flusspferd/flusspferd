# vim:ts=4:sw=4:expandtab:autoindent:
#
# The MIT License
#
# Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
#                                      http://flusspferd.org/contributors.txt)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

Include(CheckFunctionExists)
include(CheckCXXSourceCompiles)

# These tests implement the methods found in
# <http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe>

if (APPLE)
    check_function_exists(_NSGetExecutablePath FLUSSPFERD_SELF_EXE__NSGetExecutablePath)

    if(FLUSSPFERD_SELF_EXE__NSGetExecutablePath)
      add_definitions("-DFLUSSPFERD_SELF_EXE__NSGetExecutablePath")
      set(FLUSSPFERD_HAVE_SELF_EXE ON)
    endif()
endif()

if(NOT FLUSSPFERD_HAVE_SELF_EXE)
    # Linux-ism. /proc/self/exe is a symlink to the actual binary
    check_cxx_source_runs(
        "#include <unistd.h>
        #include <cstdlib>
        int main() {
          char buf[1024];
          if (readlink(\"/proc/self/exe\", buf, 1024) > 0)
            return EXIT_SUCCESS;
          else
            return EXIT_FAILURE;
        }
        "
    FLUSSPFERD_SELF_EXE_proc_self_exe)

    if (FLUSSPFERD_SELF_EXE_proc_self_exe)
        set(FLUSSPFERD_HAVE_SELF_EXE ON)
        add_definitions("-DFLUSSPFERD_SELF_EXE_proc_self_exe")
    endif()
endif()

if(NOT FLUSSPFERD_HAVE_SELF_EXE)
    # FreeBSD: sysctl CTL_KERN KERN_PROC KERN_PROC_PATHNAME -1
    check_cxx_source_runs(
        "#include <sys/types.h>
        #include <sys/sysctl.h>
        int main() {
          int mib[4];
          mib[0] = CTL_KERN;
          mib[1] = KERN_PROC;
          mib[2] = KERN_PROC_PATHNAME;
          mib[3] = -1;
          char buf[1024];
          size_t cb = sizeof(buf);
          if (sysctl(mib, 4, &cb, NULL, 0) == 0)
            return EXIT_SUCCESS;
          else
            return EXIT_FAILURE;
        }
        "
    FLUSSPFERD_SELF_EXE_sysctl_PROC_PPATHNAME)

    if (FLUSSPFERD_SELF_EXE_sysctl_PROC_PPATHNAME)
        set(FLUSSPFERD_HAVE_SELF_EXE ON)
        add_definitions("-DFLUSSPFERD_SELF_EXE_sysctl_PROC_PPATHNAME")
    endif()
endif()

if(NOT FLUSSPFERD_HAVE_SELF_EXE)
    # BSD-ism. /proc/curproc/file is a symlink to the actual binary
    check_cxx_source_runs(
        "#include <unistd.h>
        #include <cstdlib>
        int main() {
          char buf[1024];
          if (readlink(\"/proc/curproc/file\", buf, 1024) > 0)
            return EXIT_SUCCESS;
          else
            return EXIT_FAILURE;
        }
        "
    FLUSSPFERD_SELF_EXE_proc_curproc_file)

    if (FLUSSPFERD_SELF_EXE_proc_curproc_file)
        set(FLUSSPFERD_HAVE_SELF_EXE ON)
        add_definitions("-DFLUSSPFERD_SELF_EXE_proc_curproc_file")
    endif()
endif()

if(NOT FLUSSPFERD_HAVE_SELF_EXE)
    # Solaris
    check_function_exists(getexecname SELF_EXE_getexecname)

    if(SELF_EXE_getexecname)
      add_definitions("-DFLUSSPFERD_SELF_EXE_getexecname")
      set(FLUSSPFERD_HAVE_SELF_EXE ON)
    endif()
endif()

if(WIN32 AND NOT FLUSSPFERD_HAVE_SELF_EXE)
    # This might need an explicit link against kernel32.lib. Probably not
    check_function_exists(GetModuleFileName SELF_EXE_GetModuleFilename)

    if(SELF_EXE_GetModuleFilename)
      add_definitions("-DFLUSSPFERD_SELF_EXE_GetModuleFilename")
      set(FLUSSPFERD_HAVE_SELF_EXE ON)
    endif()
endif()

