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

#include <boost/system/system_error.hpp>
#include <boost/asio.hpp> // TODO split up
#include <boost/bind.hpp>

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

namespace bf = ::boost::fusion;
namespace ba = ::boost::asio; 
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

// public
namespace {
  enum { buffer_size = 4096 };
  typedef boost::array<char, buffer_size> buffer_t;
  void do_read(ba::posix::stream_descriptor &in,
               std::string &out,
               buffer_t &buf,
               boost::system::error_code const &error,
               std::size_t bytes_transferred)
  {
    if(not error) {
      out.append(buf.begin(), buf.begin() + bytes_transferred);
      ba::async_read(
        in, ba::buffer(buf),
        boost::bind(&do_read, boost::ref(in), boost::ref(out), boost::ref(buf),
                    ba::placeholders::error, ba::placeholders::bytes_transferred));
    }
    else if(error.value() == ba::error::eof) {
      if(bytes_transferred > 0) {
        out.append(buf.begin(), buf.begin() + bytes_transferred);
      }
    }
    else {
      throw boost::system::system_error(error, "while trying to read");
    }
  }

  void write_handler(boost::system::error_code const &error, std::size_t) {
    if(error) {
      throw boost::system::system_error(error, "while trying to write");
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

  buffer_t stdouttmp;
  std::string stdoutbuf;
  buffer_t stderrtmp;
  std::string stderrbuf;

  ba::io_service io_service;
  ba::posix::stream_descriptor stdind(io_service);
  if(in && !done_stdin) {
    stdind.assign(stdinfd);
    ba::async_write(stdind, ba::buffer(*in), ba::transfer_all(), write_handler);
  }

  ba::posix::stream_descriptor stdoutd(io_service);
  if(!done_stdout) {
    stdoutd.assign(stdoutfd);
    ba::async_read(
      stdoutd, ba::buffer(stdouttmp),
      boost::bind(&do_read, boost::ref(stdoutd), boost::ref(stdoutbuf), boost::ref(stdouttmp),
                  ba::placeholders::error, ba::placeholders::bytes_transferred));
  }

  ba::posix::stream_descriptor stderrd(io_service);
  if(!done_stderr) {
    stderrd.assign(stderrfd);
    ba::async_read(
      stderrd, ba::buffer(stderrtmp),
      boost::bind(&do_read, boost::ref(stderrd), boost::ref(stderrbuf), boost::ref(stderrtmp),
                  ba::placeholders::error, ba::placeholders::bytes_transferred));
  }

  io_service.run();

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
