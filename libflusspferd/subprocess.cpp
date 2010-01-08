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

#include "flusspferd/subprocess.hpp"
#include "flusspferd/exception.hpp"

#ifdef WIN32

void flusspferd::load_subprocess_module(object &ctx) {
  throw flusspferd::exception("Subprocess Module Not Yet Supported On Windows", "Error");
}

#else // POSIX/UNIX

#include "flusspferd/io/stream.hpp"
#include "flusspferd/array.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/create/object.hpp"
#include "flusspferd/create/function.hpp"
#include "flusspferd/class_description.hpp"
#include "flusspferd/property_iterator.hpp"

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#ifdef HAVE_EPOLL
#include <sys/epoll.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#endif

#include <map>
#include <vector>
#include <cstdio>
#include <iterator>
#include <algorithm>

#include "flusspferd/value_io.hpp" // DEBUG
#include <iostream> // DEBUG

namespace bio = boost::iostreams;
namespace bf = boost::fusion;
using namespace flusspferd;

namespace {
  void setnonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("fcntl: ") + std::strerror(errno_));
    }
    flags |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flags) == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("fcntl: ") + std::strerror(errno_));
    }
  }

  class fdpoll {
#ifdef HAVE_EPOLL
    int epollfd;
#else // select
    std::map<int, unsigned> fds;
#endif
  public:
    fdpoll();
    ~fdpoll();

    enum { read = 1, write = 2, error = 4 };
    void add(int fd, unsigned what);
    void remove(int fd);
    static int const indefinitely = -1;
    std::vector< std::pair<int, unsigned> > wait(int timeout_ms = indefinitely);
  };

  bool flagset(unsigned f, unsigned in) {
    return (in & f) == f;
  }

#ifdef HAVE_EPOLL
  fdpoll::fdpoll() {
    epollfd = ::epoll_create1(FD_CLOEXEC);
    if(epollfd == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("epoll_create: ") + std::strerror(errno_));
    }
  }
  fdpoll::~fdpoll() {
    close(epollfd);
  }

  void fdpoll::add(int fd, unsigned what) {
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    if(flagset(fdpoll::read, what)) {
      ev.events |= EPOLLIN;
    }
    if(flagset(fdpoll::write, what)) {
      ev.events |= EPOLLOUT;
    }
    // always waits for EPOLLERR so no need to check/set
    ev.data.fd = fd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("epoll_ctl: ") + std::strerror(errno_));
    }
  }

  void fdpoll::remove(int fd) {
    epoll_event ev;
    if(epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("epoll_ctl: ") + std::strerror(errno_));
    }
  }

  std::vector< std::pair<int, unsigned> > fdpoll::wait(int timeout_ms) {
    enum { EVENTS_N = 8 };
    epoll_event events[EVENTS_N];
    int nfds = epoll_wait(epollfd, events, EVENTS_N, timeout_ms);
    if(nfds == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("epoll_wait: ") + std::strerror(errno_));
    }
    std::vector< std::pair<int, unsigned> > ret;
    ret.reserve(nfds);
    for(int i = 0; i < nfds; ++i) {
      unsigned what = 0;
      if(flagset(EPOLLIN, events[i].events)) {
        what |= fdpoll::read;
      }
      if(flagset(EPOLLOUT, events[i].events)) {
        what |= fdpoll::write;
      }
      if(flagset(EPOLLERR, events[i].events)) {
        what |= fdpoll::error;
      }
      ret.push_back( std::make_pair(events[i].fd, what) );
    }
    return ret;
  }
#else // select
  fdpoll::fdpoll() { }
  fdpoll::~fdpoll() { }
  void fdpoll::add(int fd, unsigned what) {
    fds[fd] = what;
  }
  void fdpoll::remove(int fd) {
    fds.erase(fd);
  }
  std::vector< std::pair<int, unsigned> > fdpoll::wait(int timeout_ms) {
    fd_set rfds, wfds, efds;
    int nfds = 0;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    // TODO this is slow. maybe add a fdpoll::setup
    for(std::map<int, unsigned>::const_iterator i = fds.begin(); i != fds.end(); ++i) {
      nfds = std::max(i->first, nfds);
      if(flagset(fdpoll::read, i->second)) {
        FD_SET(i->first, &rfds);
      }
      if(flagset(fdpoll::write, i->second)) {
        FD_SET(i->first, &wfds);
      }
      if(flagset(fdpoll::error, i->second)) {
        FD_SET(i->first, &efds);
      }
    }
    timeval tv;
    if(timeout_ms != fdpoll::indefinitely) {
      tv.tv_sec = timeout_ms / 1000;
      tv.tv_usec = (timeout_ms % 1000) * 1000;
    }
    int const retfds = select(nfds+1, &rfds, &wfds, &efds,
                              timeout_ms != fdpoll::indefinitely ? &tv : 0x0);
    if(retfds == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("epoll_ctl: ") + std::strerror(errno_));
    }
    else if(retfds == 0) { // timeout
      return std::vector< std::pair<int, unsigned> >();
    }

    std::vector< std::pair<int, unsigned> > ret;
    ret.reserve(retfds);
    for(std::map<int, unsigned>::const_iterator i = fds.begin(); i != fds.end(); ++i) {
      unsigned what = 0;
      if(flagset(fdpoll::read, i->second) && FD_ISSET(i->first, &rfds)) {
        what |= fdpoll::read;
      }
      if(flagset(fdpoll::write, i->second) && FD_ISSET(i->first, &wfds)) {
        what |= fdpoll::write;
      }
      if(flagset(fdpoll::error, i->second) && FD_ISSET(i->first, &efds)) {
        what |= fdpoll::error;
      }
      ret.push_back(std::make_pair(i->first, what));
    }
    return ret;
  }
#endif
}

FLUSSPFERD_CLASS_DESCRIPTION(
  subprocess,
  (constructor_name, "subprocess")
  (full_name, "subprocess.subprocess")
  (constructible, false)
  (methods,
   ("poll", bind, poll)
   ("wait", bind, wait)
   ("communicate", bind, communicate)
   ("sendSignal", bind, send_signal)
   ("terminate", bind, terminate)
   ("kill", bind, kill))
  (properties,
   ("pid", getter, get_pid)
   ("returncode", getter, get_returncode)
   ("stdin", getter, get_stdin)
   ("stderr", getter, get_stderr)
   ("stdout", getter, get_stdout)))
{
  pid_t pid;
  bool finished;
  int returncode;

  int stdinfd;
  int stdoutfd;
  int stderrfd;

  bio::stream_buffer<bio::file_descriptor> stdinstream;
  bio::stream_buffer<bio::file_descriptor_source> stdoutstream;
  bio::stream_buffer<bio::file_descriptor_source> stderrstream;

  void close_stdin() {
    if(stdinfd != -1) {
      stdinstream.close();
      ::close(stdinfd);
      stdinfd = -1;
    }
  }

  void close_stdout() {
    if(stdoutfd != -1) {
      stdoutstream.close();
      ::close(stdoutfd);
      stdoutfd = -1;
    }
  }

  void close_stderr() {
    if(stderrfd != -1) {
      stderrstream.close();
      ::close(stderrfd);
      stderrfd = -1;
    }
  }

  value wait_impl(bool poll = false) {
    if(finished) {
      return value(returncode);
    }
    else if(pid != 0) {
      int status;
      pid_t const ret = waitpid(pid, &status, poll ? WNOHANG : 0);
      int const errno_ = errno;
      if(ret == -1) {
        throw flusspferd::exception(std::string("waitpid: ") + std::strerror(errno_));
      }
      else if(ret == 0) {
        return value(object());
      }
      else {
        if(WIFSIGNALED(status)) {
          returncode = -WTERMSIG(status);
          finished = true;
          return value(returncode);
        }
        else if(WIFEXITED(status)) {
          returncode = WEXITSTATUS(status);
          finished = true;
          return value(returncode);
        }
      }
    }
    return value(object());
  }
  bool read_impl(int fd, std::string &buffer, fdpoll &poll) { // return true if done
    enum { BUFSIZE = 2048 };
    char buf[BUFSIZE];
    ssize_t n = ::read(stdoutfd, buf, BUFSIZE);
    if(n < 0) {
      int const errno_ = errno;
      if(errno_ == EAGAIN || errno_ == EWOULDBLOCK) {
        return false;
      }
      throw flusspferd::exception(std::string("write: ") + std::strerror(errno_));
    }
    else if(n == 0) { // eof
      poll.remove(fd);
      return true; // done
    }
    buffer.append(buf, buf+n);
    return false;
  }
 public:
  subprocess(object const &self, pid_t pid, int stdinfd, int stdoutfd, int stderrfd)
    : base_type(self), pid(pid), finished(false), stdinfd(stdinfd), stdoutfd(stdoutfd),
      stderrfd(stderrfd)
  { }
  ~subprocess() {
    close_stdin();
    close_stdout();
    close_stderr();
  }

  value poll() {
    return wait_impl(true);
  }
  value wait() {
    return wait_impl(false);
  }
  object communicate(boost::optional<std::string> const &in) {
    if(in && stdinfd == -1) {
      throw flusspferd::exception("subprocess:communicate: No stdin pipe open but got input");
    }

    bool done_stdin = stdinfd == -1 || !in;
    bool done_stdout = stdoutfd == -1;
    bool done_stderr = stderrfd == -1;

    fdpoll poll;
    if(!done_stdin) {
      setnonblock(stdinfd);
      poll.add(stdinfd, fdpoll::write);
    }
    if(!done_stdout) {
      setnonblock(stdoutfd);
      poll.add(stdoutfd, fdpoll::read);
    }
    if(!done_stderr) {
      setnonblock(stderrfd);
      poll.add(stderrfd, fdpoll::read);
    }

    std::size_t bufpos = 0; // what part of `in` to write next

    std::string stdoutbuf;
    std::string stderrbuf;

    while( !done_stdin || !done_stdout || !done_stderr ) {
      std::vector< std::pair<int, unsigned> > ret = poll.wait(fdpoll::indefinitely);
      for(std::vector< std::pair<int, unsigned> >::const_iterator i = ret.begin();
          i != ret.end();
          ++i)
      {
        if(!done_stdin && i->first == stdinfd && flagset(i->second, fdpoll::write)) {
          ssize_t n = ::write(stdinfd, &(*in)[0] + bufpos, in->size() - bufpos);
          if(n < 0) {
            int const errno_ = errno;
            if(errno_ == EAGAIN || errno_ == EWOULDBLOCK) {
              continue;
            }
            else if(errno_ == EPIPE) {
              poll.remove(stdinfd);
              close_stdin();
              done_stdin = true;
              continue;
            }
            throw flusspferd::exception(std::string("write: ") + std::strerror(errno_));
          }
          bufpos += std::size_t(n);
          if(bufpos >= in->size()) {
            poll.remove(stdinfd);
            done_stdin = true;
          }
        }
        else if(!done_stdout && i->first == stdoutfd && flagset(i->second, fdpoll::read)) {
          done_stdout = read_impl(stdoutfd, stdoutbuf, poll);
        }
        else if(!done_stderr && i->first == stderrfd && flagset(i->second, fdpoll::read)) {
          done_stderr = read_impl(stderrfd, stderrbuf, poll);
        }
      }
    }

    value rc = wait();

    object ret = flusspferd::create<flusspferd::object>();
    ret.define_property("returncode", rc);
    ret.define_property("stdout", value(stdoutbuf));
    ret.define_property("stderr", value(stderrbuf));

    return ret;
  }
  void send_signal(int sig) {
    if(pid != 0) {
      if(::kill(pid, sig) == -1) {
        int const errno_ = errno;
        throw flusspferd::exception(std::string("kill: ") + std::strerror(errno_));
      }
    }
  }
  void terminate() {
    send_signal(SIGTERM);
  }
  void kill() {
    send_signal(SIGKILL);
  }
  int get_pid() { return static_cast<int>(pid); }
  value get_returncode() {
    if(finished) {
      return value(returncode);
    }
    else {
      return value(object());
    }
  }
  object get_stdin()  {
    if(stdinfd == -1) {
      return object();
    }
    else {
      stdinstream.open(bio::file_descriptor(stdinfd));
      return flusspferd::create<io::stream>(bf::vector1<std::streambuf*>(&stdinstream));
    }
  }
  object get_stderr() { return object(); } // TODO
  object get_stdout() { return object(); } // TODO

  static subprocess &create(pid_t p, int in, int out, int err) {
    return flusspferd::create<subprocess>(boost::fusion::make_vector(p, in, out, err));
  }
};

namespace {
  void do_pipe(int pipefd[2]) {
    if(::pipe(pipefd) == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("pipe: ") + std::strerror(errno_));
    }
  }

  subprocess &do_popen(char const *cmd,
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
      throw flusspferd::exception(std::string("fork: ") + std::strerror(errno_));
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
    return subprocess::create(pid, stdinp[1], stdoutp[0], stderrp[0]);
  }

  std::vector<char const*> array2args(object o) {
    assert(o.is_array());
    array a(o);
    std::vector<char const*> args;
    array::iterator const end = a.end();
    for(array::iterator i = a.begin(); i != end; ++i) {
      if(!i->is_string()) {
        throw flusspferd::exception("popen array elements must be strings",
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
}

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

 */

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
          throw flusspferd::exception("popen expectes Array args property", "TypeError");
        }
        std::vector<char const*> args = array2args(o.get_property("args").get_object());

        // executable
        char const *cmd = 0x0;
        if(o.has_property("executable") && o.get_property("executable").is_string()) {
          cmd = o.get_property("executable").get_string().c_str();
        }

        // shell TODO
        bool shell = false;

        // cwd TODO
        char const *cwd = 0x0;

        // stdin, stderr, stdout TODO
        bool stdin_ = true;
        bool stderr_ = true;
        bool stdout_ = true;

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
      throw flusspferd::exception("popen: expects object or string as parameter", "TypeError");
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
      else if(!(x.arg[1].get_string() == "r")) {
        throw flusspferd::exception("popen: second parameter should be \"r\" or \"w\"");
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
        throw flusspferd::exception("popen: second parameter should be \"r\" or \"w\"");
      }

      x.result = do_popen(0x0, args, env, in, !in, false);
      return;
    }
    else {
      throw flusspferd::exception("popen: got wrong parameter type. Expected string,string or array,string", "TypeError");
    }
  }
  else {
    throw flusspferd::exception("popen: wrong number of parameters");
  }
}

void flusspferd::load_subprocess_module(object &ctx) {
  object exports = ctx.get_property_object("exports");

  flusspferd::create<flusspferd::function>(
    "popen", &Popen,
    param::_container = exports);
  load_class<subprocess>(exports);
  exports.define_properties(read_only_property | permanent_property)
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
    ("SIGPOLL", value(static_cast<int>(SIGPOLL))) // Pollable event. (T)
    ("SIGPROF", value(static_cast<int>(SIGPROF))) // Profiling timer expired. (T)
    ("SIGSYS", value(static_cast<int>(SIGSYS))) // Bad system call. (A)
    ("SIGTRAP", value(static_cast<int>(SIGTRAP))) // Trace/breakpoint trap. (A)
    ("SIGURG", value(static_cast<int>(SIGURG))) // High bandwidth data is available at a socket. (I)
    ("SIGVTALRM", value(static_cast<int>(SIGVTALRM))) // Virtual timer expired. (T)
    ("SIGXCPU", value(static_cast<int>(SIGXCPU))) // CPU time limit exceeded. (A)
    ("SIGXFSZ", value(static_cast<int>(SIGXFSZ))); // File size limit exceeded. (A)
}

#endif
