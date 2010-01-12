/*
The MIT License

Copyright (c) 2010 Flusspferd contributors (see "CONTRIBUTORS" or
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
#ifndef FLUSSPFERD_PLUGIN_SUBPROCESS_POSIX_POPEN_HPP
#define FLUSSPFERD_PLUGIN_SUBPROCESS_POSIX_POPEN_HPP

#ifndef HAVE_POSIX
#error "POSIX support required"
#endif

namespace flusspferd {
  class call_context;
}

namespace subprocess {
  /*
    call style:

    "w","r"

    popen("foo", "w"); // popen(3)

    popen(["foo", "arg0", "arg1"], "r"); // similar to popen(3) but uses execve instead of shell

    popen("foo"); // runs cmd in shell and opens stdin/stdout/stderr
    popen([...]);

    popen(obj);
    obj = {
    args : Array containing the Arguments (args[0] is used as executable if .executable is null!,
    executable : optString - The name of the executable,
    shell : optBoolean - true -> use shell to invoke program or false -> call execve? (default: false)
    cwd : optString - Working Directory
    env : optEnumeratableObject containing the environment (default: system.env)
    stdin : optBoolean - true -> open pipe for stdin (default: true)
    stdout : optBoolean
    stderr : optBoolean
    };
    if shell is true then args[0] ist passed to the shell and the rest as arguments to the shell(!)

    TODO stdin/stdout/stderr could also be existing streams

  */

  void Popen(flusspferd::call_context &x);
}

#endif
