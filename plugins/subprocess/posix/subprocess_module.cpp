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
#include "flusspferd/modules.hpp"

#include "flusspferd/create.hpp"
#include "flusspferd/create/function.hpp"

#include "subprocess.hpp"
#include "popen.hpp"

#include <signal.h>

using namespace flusspferd;

FLUSSPFERD_LOADER_SIMPLE(subprocess_) {
  create<flusspferd::function>(
    "popen", &subprocess::Popen,
     param::_container = subprocess_);
  load_class<subprocess::Subprocess>(subprocess_);
  subprocess_.define_properties(read_only_property | permanent_property)
    ("SIGABRT", value(static_cast<int>(SIGABRT))) // Process abort signal. (A)
    ("SIGALRM", value(static_cast<int>(SIGALRM))) // Alarm clock. (T)
    ("SIGBUS", value(static_cast<int>(SIGBUS))) // Access to an undefined portion of a memory object. (A)
    ("SIGCHLD", value(static_cast<int>(SIGCHLD))) // Child process terminated, stopped, or continued. (I)
    ("SIGCONT", value(static_cast<int>(SIGCONT))) // Continue executing, if stopped. (C)
    ("SIGFPE", value(static_cast<int>(SIGFPE))) // Erroneous arithmetic operation. (A)
    ("SIGHUP", value(static_cast<int>(SIGHUP))) // Hangup. (T)
    ("SIGILL", value(static_cast<int>(SIGILL))) // Illegal instruction. (A)
    ("SIGINT", value(static_cast<int>(SIGINT))) // Terminal interrupt signal. (T)
    ("SIGKILL", value(static_cast<int>(SIGKILL))) // Kill (cannot be caught or ignored). (T)
    ("SIGPIPE", value(static_cast<int>(SIGPIPE))) // Write on a pipe with no one to read it. (T)
    ("SIGQUIT", value(static_cast<int>(SIGQUIT))) // Terminal quit signal. (A)
    ("SIGSEGV", value(static_cast<int>(SIGSEGV))) // Invalid memory reference. (A)
    ("SIGSTOP", value(static_cast<int>(SIGSTOP))) // Stop executing (cannot be caught or ignored). (S)
    ("SIGTERM", value(static_cast<int>(SIGTERM))) // Termination signal. (T)
    ("SIGTSTP", value(static_cast<int>(SIGTSTP))) // Terminal stop signal. (S)
    ("SIGTTIN", value(static_cast<int>(SIGTTIN))) // Background process attempting read. (S)
    ("SIGTTOU", value(static_cast<int>(SIGTTOU))) // Background process attempting write. (S)
    ("SIGUSR1", value(static_cast<int>(SIGUSR1))) // User-defined signal 1. (T)
    ("SIGUSR2", value(static_cast<int>(SIGUSR2))) // User-defined signal 2. (T)
#ifdef SIGPOLL
    ("SIGPOLL", value(static_cast<int>(SIGPOLL))) // Pollable event. (T)
#endif
    ("SIGPROF", value(static_cast<int>(SIGPROF))) // Profiling timer expired. (T)
    ("SIGSYS", value(static_cast<int>(SIGSYS))) // Bad system call. (A)
    ("SIGTRAP", value(static_cast<int>(SIGTRAP))) // Trace/breakpoint trap. (A)
    ("SIGURG", value(static_cast<int>(SIGURG))) // High bandwidth data is available at a socket. (I)
    ("SIGVTALRM", value(static_cast<int>(SIGVTALRM))) // Virtual timer expired. (T)
    ("SIGXCPU", value(static_cast<int>(SIGXCPU))) // CPU time limit exceeded. (A)
    ("SIGXFSZ", value(static_cast<int>(SIGXFSZ))); // File size limit exceeded. (A)
}
