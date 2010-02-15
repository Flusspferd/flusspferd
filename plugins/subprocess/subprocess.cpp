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

#include <flusspferd/io/stream.hpp>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#if defined(BOOST_POSIX_API)
# include <boost/process/posix_status.hpp>
typedef boost::asio::posix::stream_descriptor asio_stream;
#elif defined(BOOST_WINDOWS_API)
# include <boost/process/win32_child.hpp>
typedef boost::asio::windows::stream_handle asio_stream;
#else
# error Unsupported platform
#endif

#include <boost/bind.hpp>

namespace ba = ::boost::asio;

using namespace flusspferd;
using namespace subprocess;
using boost::optional;
using boost::system::error_code;

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
      error_code(errno, boost::system::get_system_category()),
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
  if ( has_own_property("returncode") )
    return get_property("returncode");

  // Stupidly, Boost.Process doesn't have an option to do non blocking, even
  // tho its trivially easy:

  boost::optional<bp::status> s = child_.wait(poll);

  if (!s)
    return object();

  value ret;
#if defined(BOOST_POSIX_API)

  bp::posix_status p(s.get());
  // Stupid thing is protected. grr.
  //bp::posix_status status(s);

  if (p.signaled())
    ret = value(-p.term_signal());
  else
#endif
  if (s->exited())
    ret = value(s->exit_status());
  else
    // This is unlikely to happen
    ret = object();

  define_property("returncode", ret, read_only_property | permanent_property );
  return ret;
}

namespace {
  ba::io_service &get_io_service() {
    // TODO: ideally this wouldn't use its own io_service if there is one in the process already.
    static ba::io_service svc;
    return svc;
  }
}

void Subprocess::handle_write( error_code const &ec, bool &done ) {
  if (ec)
    throw boost::system::system_error(ec, "Subprocess: writing to stdin");

  done = true;
}

namespace {
  struct reader_state : public boost::enable_shared_from_this<reader_state> {
    boost::array<char, 4096> buff;
    bool &done;
    boost::scoped_ptr<asio_stream> s;
    string str;
    object o;
    std::string prop;

    reader_state(bp::pistream &s_, ba::io_service &svc, bool &done_, object o_, std::string const &prop_)
      : done(done_),
        s(new asio_stream(svc)),
        o(o_),
        prop(prop_)
    {
      s->assign( s_.handle().get() );
      o.set_property(prop, str);
      done = false;
    }

    void handle_read( error_code const &ec, std::size_t n_read ) {
      if (n_read) {
        str = string::concat( str, string(buff.data(), n_read) );
        o.set_property(prop, str);
      }

      // OSX gives EoF, Win32 gives EPIPE error.
      if (ec == ba::error::eof || ec == ba::error::broken_pipe) {
        s->close();
        done = true;
      }
      else if (ec)
        throw boost::system::system_error(ec, std::string("Subprocess: reading from ") + prop + " pipe");
      else
        enqueue();
    }


    void enqueue() {
      ba::async_read(
        *s, ba::buffer(buff),
        boost::bind( &reader_state::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred )
      );
    }
  };
}

object Subprocess::communicate( optional<value> input_ ) {

  // stupid non-copyable root<T> objects. Right PITA they are.
  boost::scoped_ptr<root_string> root_input;

  ba::io_service &svc = get_io_service();

  bool done_stdout = true, done_stderr = true, done_stdin = true;

  if ( input_ ) {
    if ( stdin_ && stdin_->is_undefined() )
      throw exception("Subprocess#communicate: input provided when child's stdin is closed");

    done_stdin = false;

    // We need the string to be rooted until the async_write compeltes.
    root_input.reset( new root_string( input_->to_string() ) );
    std::string const &s = root_input->to_string();

    asio_stream in( svc );
    in.assign( child_.get_stdin().handle().release() );

    ba::async_write( in, ba::buffer( s.data(), s.size() ),
      boost::bind( &Subprocess::handle_write, this, ba::placeholders::error, done_stdin )
    );
  }

  object ret = create<object>();
  root_object root_ret(ret);

  if ( !stdout_ || !stdout_->is_undefined() ) {
    boost::shared_ptr<reader_state> r(new reader_state(child_.get_stdout(), svc, done_stdout, ret, "stdout"));
    r->enqueue();
  }
  else {
    ret.set_property("stdout", object());
  }

  if ( !stderr_ || !stderr_->is_undefined() ) {
    boost::shared_ptr<reader_state> r(new reader_state(child_.get_stderr(), svc, done_stderr, ret, "stderr"));
    r->enqueue();
  }
  else {
    ret.set_property("stderr", object());
  }

  while (!done_stderr || !done_stdout || !done_stderr) {
    svc.run();
  }
  svc.reset();

  ret.set_property("returncode", wait());

  return ret;
}
