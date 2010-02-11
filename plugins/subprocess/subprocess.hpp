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
#ifndef FLUSSPFERD_PLUGIN_SUBPROCESS_HPP
#define FLUSSPFERD_PLUGIN_SUBPROCESS_HPP

#include "flusspferd/tracer.hpp"
#include "flusspferd/create/object.hpp"
#include "flusspferd/class_description.hpp"

#include <boost/process/child.hpp>
#include <boost/process/context.hpp>

namespace subprocess {
  namespace bp = boost::process;

FLUSSPFERD_CLASS_DESCRIPTION(
  Subprocess,
  (constructor_name, "Subprocess")
  (full_name, "subprocess.Subprocess")
  (constructible, false)
  (methods,
    ("terminate", bind, terminate)
    ("kill", bind, kill)
    ("sendSignal", bind, send_signal)
    ("wait", bind, wait)
    ("poll", bind, poll)
  )
  (properties,
    ("pid", getter, get_pid)
    ("stdin", getter, get_stdin)
    ("stdout", getter, get_stdout)
    ("stderr", getter, get_stderr)
  )
) {
  protected:
    bp::child child_;
    bp::context ctx_;

    boost::optional<flusspferd::value> stdin_, stdout_, stderr_;

    void trace( flusspferd::tracer &trc );

    // Helper method for get_std*
    template<class T> flusspferd::value 
    get_stream(boost::optional<flusspferd::value>&s, T& (bp::child::* getter)() const);

    flusspferd::value wait_impl(bool poll);
  public:
    Subprocess( flusspferd::object const &o, bp::child child, bp::context ctx );

    int get_pid() {
      return child_.get_id();
    }

    flusspferd::value get_stdin();
    flusspferd::value get_stdout();
    flusspferd::value get_stderr();

    void terminate();
    void kill();
    void send_signal(int sig);

    flusspferd::value wait() { return wait_impl(false); }
    flusspferd::value poll() { return wait_impl(true); }

    flusspferd::object communicate( boost::optional<flusspferd::value> stdin_ );
};

} // namespace subprocess

#endif

