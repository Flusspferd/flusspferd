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
#include "exception.hpp"

#include <flusspferd/io/stream.hpp>

#if defined(BOOST_POSIX_API)
# include <boost/process/posix_status.hpp>
#elif defined(BOOST_WINDOWS_API) 
# include <boost/process/win32_child.hpp>
#else
# error Unsupported platform
#endif

#include <boost/bind.hpp>

namespace bp = ::boost::process; 

using namespace flusspferd;
using namespace subprocess;
using boost::optional;

Subprocess::Subprocess( object const& o, bp::child child, bp::context ctx )
  : base_type(o),
    child_(child)
{
  // If any of the streams are closed, store undefined in the slots
  if ( ctx.stdin_behavior.get_type() != bp::stream_behavior::capture )
    stdin_.reset( value() );
  if ( ctx.stdout_behavior.get_type() != bp::stream_behavior::capture )
    stdout_.reset( value() );
  if ( ctx.stderr_behavior.get_type() != bp::stream_behavior::capture )
    stderr_.reset( value() );
}

void Subprocess::trace( tracer &trc ) {
  if ( stdin_ )
    trc( "Subprocess#stdin", *stdin_ );
  if ( stdout_ )
    trc( "Subprocess#stdout", *stdout_ );
  if ( stderr_ )
    trc( "Subprocess#stderr", *stderr_ );
}


template<class T>
value Subprocess::get_stream( boost::optional<value> &s, T& (bp::child::* getter)() const ) {
  namespace bf = boost::fusion;

  if ( !s ) {
    T &stream = (child_.*getter)();
    object o = create<io::stream>( bf::vector1<std::streambuf*>(
      stream.rdbuf()
    ) );

    create<function>(
      "close",
      boost::bind(&T::close, &stream),
      param::_container = o
    );

    s.reset( o );
  }
  return *s;
  //in.assign(.handle().release()); 
}

value Subprocess::get_stdin() {
  return get_stream( stdin_, &bp::child::get_stdin );
}

value Subprocess::get_stdout() {
  return get_stream( stdout_, &bp::child::get_stdout );
}

value Subprocess::get_stderr() {
  return get_stream( stderr_, &bp::child::get_stderr );
}


void Subprocess::terminate() {
  // This might want to do something else on Win32 - this calls
  // TerminateProcess right now which is akin to kill -9. i.e. uncatchable.
  child_.terminate(false);
}

void Subprocess::kill() {
  child_.terminate(true);
}

#include <boost/system/system_error.hpp>
void Subprocess::send_signal(int sig) {
#ifdef BOOST_POSIX_API
  if (::kill(child_.get_id(), sig) == -1) 
    throw boost::system::system_error(
      boost::system::error_code(errno, boost::system::get_system_category()),
      "Subprocess#sendSignal"
    );
#else
  if (sig == 15)
    terminate();
  else if (sig == 9)
    kill();
  else
    throw exception("Subprocess#sendSignal: unsupported signal value on this platform");
#endif
}

value Subprocess::wait_impl(bool poll) {
  // Stupidly, Boost.Process doesn't have an option to do non blocking, even
  // tho its trivially easy:

  value ret;
  using namespace boost::system;
#if defined(BOOST_POSIX_API) 
  int s; 
  pid_t wait = ::waitpid(child_.get_id(), &s, poll ? WNOHANG : 0);
  
  if (wait == -1) 
      boost::throw_exception(system_error(error_code(errno, get_system_category()), "Subprocess: waitpid(2) failed")); 

  if (wait == 0)
    return object();

  // Stupid thing is protected. grr.
  //bp::posix_status status(s);

  else if (WIFSIGNALED(s))
    ret = value(-WTERMSIG(s));
  else if (WIFEXITED(s))
    ret = value(WEXITSTATUS(s));
  else
    ret = object();

#elif defined(BOOST_WINDOWS_API) 
  DWORD ret = ::WaitForSingleObject(child_.get_process_handle(), poll? 0 : INFINITE);
  
  if (ret == WAIT_FAILED)
      boost::throw_exception(system_error(error_code(::GetLastError(), get_system_category()), "Subprocess: WaitForSingleObject failed")); 

  if (ret == WAIT_TIMEOUT)
    ret = object();
  else {

    DWORD code; 
    if ( !GetExitCodeProcess(child_.get_process_handle(), &code) )
        boost::throw_exception(system_error(error_code(::GetLastError(), get_system_category()), "Subprocess: GetExitCodeProcess failed")); 

    ret = value(code);
  }
#endif

  define_property("returncode", ret, read_only_property | permanent_property );
  return ret;
}

object Subprocess::communicate( optional<value> stdin_ ) {

}
