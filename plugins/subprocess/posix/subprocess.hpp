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
#ifndef FLUSSPFERD_PLUGIN_SUBPROCESS_POSIX_SUBPROCESS_HPP
#define FLUSSPFERD_PLUGIN_SUBPROCESS_POSIX_SUBPROCESS_HPP

#ifndef HAVE_POSIX
#error "POSIX support required"
#endif

#include "flusspferd/tracer.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/io/stream.hpp"
#include "flusspferd/create/object.hpp"
#include "flusspferd/class_description.hpp"

#include "errno.hpp"

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <string>

#include <sys/types.h>
#include <signal.h>

namespace subprocess {
  namespace bio = boost::iostreams;

  FLUSSPFERD_CLASS_DESCRIPTION(
    Subprocess,
    (constructor_name, "Subprocess")
    (full_name, "subprocess.Subprocess")
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

    bio::stream_buffer<bio::file_descriptor_sink> stdinstreambuf;
    flusspferd::io::stream *stdinstream;
    bio::stream_buffer<bio::file_descriptor_source> stdoutstreambuf;
    flusspferd::io::stream *stdoutstream;
    bio::stream_buffer<bio::file_descriptor_source> stderrstreambuf;
    flusspferd::io::stream *stderrstream;

    void close_stdin();
    void close_stdout();
    void close_stderr();
    flusspferd::value wait_impl(bool poll = false);
  protected:
    void trace(flusspferd::tracer &trc) {
      if(stdinstream) {
        trc("Subprocess#stdin", *stdinstream);
      }
      if(stdoutstream) {
        trc("Subprocess#stdout", *stdoutstream);
      }
      if(stderrstream) {
        trc("Subprocess#stderr", *stderrstream);
      }
    }

  public:
    Subprocess(object const &self, pid_t pid, int stdinfd, int stdoutfd, int stderrfd)
      : base_type(self), pid(pid), finished(false), stdinfd(stdinfd), stdoutfd(stdoutfd),
        stderrfd(stderrfd), stdinstream(0x0), stdoutstream(0x0), stderrstream(0x0)
      { }
    ~Subprocess() {
      close_stdin();
      close_stdout();
      close_stderr();
    }

    flusspferd::value poll() {
      return wait_impl(true);
    }
    flusspferd::value wait() {
      return wait_impl(false);
    }
    flusspferd::object communicate(boost::optional<std::string> const &in);

    void send_signal(int sig);
    void terminate() {
      send_signal(SIGTERM);
    }
    void kill() {
      send_signal(SIGKILL);
    }
    int get_pid() { return static_cast<int>(pid); }
    flusspferd::value get_returncode() {
      if(finished) {
        return flusspferd::value(returncode);
      }
      else {
        return flusspferd::value(object());
      }
    }
    flusspferd::value get_stdin();
    flusspferd::value get_stdout();
    flusspferd::value get_stderr();

    static Subprocess &create(pid_t p, int in, int out, int err) {
      return flusspferd::create<Subprocess>(boost::fusion::make_vector(p, in, out, err));
    }
  };
}

#endif
