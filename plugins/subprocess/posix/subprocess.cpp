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

#include "subprocess.hpp"

#include "errno.hpp"
#include "fdpoll.hpp"

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

namespace bf = boost::fusion;
using namespace flusspferd;

namespace subprocess {
  // private
void Subprocess::close_stdin() {
  if(stdinfd != -1) {
    if(stdinstreambuf.is_open()) {
      stdinstreambuf.close();
    }
    ::close(stdinfd);
    stdinfd = -1;
  }
}

void Subprocess::close_stdout() {
  if(stdoutfd != -1) {
    if(stdoutstreambuf.is_open()) {
      stdoutstreambuf.close();
    }
    ::close(stdoutfd);
    stdoutfd = -1;
  }
}

void Subprocess::close_stderr() {
  if(stderrfd != -1) {
    if(stderrstreambuf.is_open()) {
      stderrstreambuf.close();
    }
    ::close(stderrfd);
    stderrfd = -1;
  }
}

value Subprocess::wait_impl(bool poll) {
  if(finished) {
    return value(returncode);
  }
  else if(pid != 0) {
    int status;
    pid_t const ret = waitpid(pid, &status, poll ? WNOHANG : 0);
    int const errno_ = errno;
    if(ret == -1) {
      throw subprocess::exception("waitpid") << subprocess::errno_info(errno_);
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
bool Subprocess::read_impl(int fd, std::string &buffer, fdpoll &poll) {
  enum { BUFSIZE = 2048 };
  char buf[BUFSIZE];
  ssize_t n = ::read(stdoutfd, buf, BUFSIZE);
  if(n < 0) {
    int const errno_ = errno;
    if(errno_ == EAGAIN || errno_ == EWOULDBLOCK) {
      return false;
    }
    throw subprocess::exception("read") << subprocess::errno_info(errno_);
  }
  else if(n == 0) { // eof
    poll.remove(fd);
    return true; // done
  }
  buffer.append(buf, buf+n);
  return false;
}
  
  // public

namespace {
  void setnonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1) {
      int const errno_ = errno;
      throw subprocess::exception("fcntl") << subprocess::errno_info(errno_);
    }
    flags |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flags) == -1) {
      int const errno_ = errno;
      throw subprocess::exception("fcntl") << subprocess::errno_info(errno_);
    }
  }
}

object Subprocess::communicate(boost::optional<std::string> const &in) {
  if(in && stdinfd == -1) {
    throw subprocess::exception("Subprocess#communicate: No stdin pipe open but got input");
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
            throw subprocess::exception("write") << subprocess::errno_info(errno_);
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

void Subprocess::send_signal(int sig) {
  if(pid != 0) {
    if(::kill(pid, sig) == -1) {
      int const errno_ = errno;
      throw subprocess::exception("kill") << subprocess::errno_info(errno_);
    }
  }
}

value Subprocess::get_stdin()  {
  if(stdinfd == -1 && !stdinstream) {
    return value();
  }
  else {
    if(!stdinstream) {
      if(!stdinstreambuf.is_open()) {
        stdinstreambuf.open(bio::file_descriptor_sink(stdinfd));
      }
      stdinstream = &flusspferd::create<io::stream>(
        bf::vector1<std::streambuf*>(&stdinstreambuf));
    }
    return *stdinstream;
  }
}

value Subprocess::get_stdout() {
  if(stdoutfd == -1 && !stdoutstream) {
    return value();
  }
  else {
    if(!stdoutstream) {
      if(!stdoutstreambuf.is_open()) {
        stdoutstreambuf.open(bio::file_descriptor_source(stdoutfd));
      }
      stdoutstream = &flusspferd::create<io::stream>(
        bf::vector1<std::streambuf*>(&stdoutstreambuf));
    }
    return *stdoutstream;
  }
}

value Subprocess::get_stderr() {
  if(stderrfd == -1 && !stderrstream) {
    return value();
  }
  else {
    if(!stderrstream) {
      if(!stderrstreambuf.is_open()) {
        stderrstreambuf.open(bio::file_descriptor_source(stderrfd));
      }
      stderrstream = &flusspferd::create<io::stream>(
        bf::vector1<std::streambuf*>(&stderrstreambuf));
    }
    return *stderrstream;
  }
}
}
