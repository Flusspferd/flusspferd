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
#include "popen.hpp"

#include "errno.hpp"
#include "subprocess.hpp"

#include "flusspferd/array.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/call_context.hpp"

#include <vector>
#include <string>

#include <unistd.h>

using namespace flusspferd;
using namespace subprocess;

namespace {
  void do_pipe(int pipefd[2]) {
    if(::pipe(pipefd) == -1) {
      int const errno_ = errno;
      throw subprocess::exception("pipe") << subprocess::errno_info(errno_);
    }
  }

  Subprocess &do_popen(char const *cmd,
                       std::vector<char const*> const &args,
                       std::vector<char const*> const &env,
                       bool stdin_, bool stdout_, bool stderr_,
                       char const *cwd = 0x0)
  {
    assert(!args.empty() && args.back() == 0x0);
    assert(env.empty() || env.back() == 0x0);
    if(!cmd) {
      cmd = args[0];
    }

    int stdinp[2];
    if(stdin_) {
      do_pipe(stdinp);
    }
    int stdoutp[2];
    if(stdout_) {
      do_pipe(stdoutp);
    }
    int stderrp[2];
    if(stderr_) {
      do_pipe(stderrp);
    }

    pid_t pid = fork(); // could use vfork!
    if(pid == -1) {
      int const errno_ = errno;
      throw subprocess::exception("fork") << subprocess::errno_info(errno_);
    }
    else if(pid == 0) {
      if(stdin_) {
        close(stdinp[1]);
        if(stdinp[0] != STDIN_FILENO) {
          dup2(stdinp[0], STDIN_FILENO);
          close(stdinp[0]);
        }
      }
      if(stdout_) {
        int tp1 = stdoutp[1];
        close(stdoutp[0]);
        if(tp1 != STDOUT_FILENO) {
          dup2(tp1, STDOUT_FILENO);
          close(tp1);
          tp1 = STDOUT_FILENO;
        }
      }
      if(stderr_) {
        int tp1 = stderrp[1];
        close(stderrp[0]);
        if(tp1 != STDERR_FILENO) {
          dup2(tp1, STDERR_FILENO);
          close(tp1);
          tp1 = STDERR_FILENO;
        }
      }
      if(cwd) {
        chdir(cwd);
      }
      // TODO environ stuff!
      execvp(cmd, const_cast<char *const*>(&args[0]));
      _exit(127);
    }

    if(stdin_) {
      close(stdinp[0]);
    }
    else {
      stdinp[1] = -1;
    }

    if(stdout_) {
      close(stdoutp[1]);
    }
    else {
      stdoutp[0] = -1;
    }

    if(stderr_) {
      close(stderrp[1]);
    }
    else {
      stderrp[0] = -1;
    }
    return Subprocess::create(pid, stdinp[1], stdoutp[0], stderrp[0]);
  }

  std::vector<char const*> array2args(object o) {
    assert(o.is_array());
    array a(o);
    std::vector<char const*> args;
    array::iterator const end = a.end();
    for(array::iterator i = a.begin(); i != end; ++i) {
      if(!i->is_string()) {
        throw subprocess::exception("popen array elements must be strings",
                                    "TypeError");
      }
      args.push_back(i->get_string().c_str());
    }
    args.push_back(0x0);
    return args;
  }

  std::vector<char const*> shellargs(value v) {
    assert(v.is_string());
    std::vector<char const*> args;
    args.push_back("sh");
    args.push_back("-c");
    args.push_back(v.get_string().c_str());
    args.push_back(0x0);
    return args;
  }

  void getbool(object &o, char const *name, bool &p) {
    if(o.has_property(name)) {
      if(!o.get_property(name).is_boolean()) {
        throw subprocess::exception(std::string("popen expected `") + name + "' to be a boolean",
                                    "TypeError");
      }
      p = o.get_property(name).get_boolean();
    }
  }

  char const *getcstring(object &o, char const *name) {
    if(o.has_property(name)) {
      if(!o.get_property(name).is_string()) {
        throw subprocess::exception(std::string("popen expected `") + name + "' to be a string",
                                    "TypeError");
      }
      return o.get_property(name).get_string().c_str();
    }
    return 0x0;
  }
}

namespace subprocess {
void Popen(flusspferd::call_context &x) {
  if(x.arg.size() == 1) {
    if(x.arg.front().is_object()) {
      if(x.arg.front().get_object().is_array()) {
        std::vector<char const*> args = array2args(x.arg[0].get_object());       
        std::vector<char const*> env;
        x.result = do_popen(0x0, args, env, true, true, true);
        return;
      }
      else {
        object o = x.arg[0].get_object();
        // args
        if(!o.has_property("args") ||
           !o.get_property("args").is_object() ||
           !o.get_property("args").get_object().is_array())
        {
          throw subprocess::exception("popen expects args (Array) property", "TypeError");
        }
        std::vector<char const*> args = array2args(o.get_property("args").get_object());

        // executable
        char const *cmd = getcstring(o, "executable");

        // shell
        bool shell = false;
        getbool(o, "shell", shell);

        // cwd
        char const *cwd = getcstring(o, "cwd");

        // stdin, stderr, stdout
        bool stdin_ = true;
        getbool(o, "stdin", stdin_);
        bool stderr_ = true;
        getbool(o, "stderr", stderr_);
        bool stdout_ = true;
        getbool(o, "stdout", stdout_);

        // env TODO
        std::vector<char const*> env;

        if(shell) {
          std::vector<char const*> shellargs;
          shellargs.reserve(2 + args.size());
          shellargs.push_back("sh");
          shellargs.push_back("-c");
          std::copy(args.begin(), args.end(), std::back_inserter(shellargs));
          x.result = do_popen(0x0, shellargs, env, stdin_, stderr_, stdout_, cwd);
        }
        else {
          x.result = do_popen(cmd, args, env, stdin_, stderr_, stdout_, cwd);
        }
        return;
      }
    }
    else if(x.arg.front().is_string()) {
      std::vector<char const*> args = shellargs(x.arg.front());
      std::vector<char const*> env;
      x.result = do_popen(0x0, args, env, true, true, true);
      return;
    }
    else {
      throw subprocess::exception("popen expects object or string as parameter", "TypeError");
    }
  }
  else if(x.arg.size() == 2) {
    if(x.arg[0].is_string() && x.arg[1].is_string()) {
      std::vector<char const*> args = shellargs(x.arg[0]);
      std::vector<char const*> env;

      bool in = false;
      if(x.arg[1].get_string() == "w") {
        in = true;
      }
      else if(x.arg[1].get_string() != "r") {
        throw subprocess::exception("popen: second parameter should be \"r\" or \"w\"");
      }

      x.result = do_popen(0x0, args, env, in, !in, false);
      return;
    }
    else if(x.arg[0].is_object() && x.arg[0].get_object().is_array() && x.arg[1].is_string()) {
      std::vector<char const*> args = array2args(x.arg[0].get_object());
      std::vector<char const*> env;

      bool in = false;
      if(x.arg[1].get_string() == "w") {
        in = true;
      }
      else if(!(x.arg[1].get_string() == "r")) {
        throw subprocess::exception("popen: second parameter should be \"r\" or \"w\"");
      }

      x.result = do_popen(0x0, args, env, in, !in, false);
      return;
    }
    else {
      throw subprocess::exception(
        "popen: got wrong parameter type. Expects string,string or array,string", "TypeError");
    }
  }
  else {
    throw subprocess::exception("popen: wrong number of parameters");
  }
}
}
