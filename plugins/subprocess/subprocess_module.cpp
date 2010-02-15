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
#define _SCL_SECURE_NO_DEPRECATE
#include "flusspferd/modules.hpp"

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/create/function.hpp"
#include "flusspferd/property_iterator.hpp"

#include "subprocess.hpp"

#ifndef WIN32
# include <signal.h>
#else
# define BOOST_PROCESS_WINDOWS_USE_NAMED_PIPE
#endif

#include <boost/format.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/optional.hpp>
#include <boost/process.hpp>
#include <boost/system/error_code.hpp>


namespace bp = ::boost::process;
namespace bs = ::boost::system;
using boost::optional;
using boost::format;
using boost::tribool;
using boost::indeterminate;


using namespace flusspferd;

namespace subprocess {
  void popen(flusspferd::call_context &x);
}

FLUSSPFERD_LOADER_SIMPLE(exports) {
  create<flusspferd::function>(
    "popen", &subprocess::popen,
     param::_container = exports);

  load_class<subprocess::Subprocess>(exports);

  exports.define_properties(read_only_property | permanent_property)
#ifdef WIN32
    //("SIGINT", 2) // Terminal interrupt signal. (T)
    ("SIGTERM", 15) // Termination signal. (T)
    ("SIGKILL", 9) // Kill (cannot be caught or ignored). (T)
#else
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
    ("SIGXFSZ", value(static_cast<int>(SIGXFSZ))) // File size limit exceeded. (A)
#endif
    ;
}

namespace {
  std::vector<std::string> array_to_vector(array a) {
    std::vector<std::string> args;

    array::iterator const end = a.end();

    for(array::iterator i = a.begin(); i != end; ++i) {
      std::string s = i->to_std_string();
      args.push_back( std::string( s.data(), s.data() + s.size() ) );
    }
    return args;
  }

  void apply_mode_string( bp::context &ctx, std::string const &mode ) {

    if (mode != "") {
      // This argform doesn't get stderr. Its an open stream, it just doesn't go
      // anywhere (i.e. /dev/null or similar)
      ctx.stderr_behavior = bp::silence_stream();
    }

    if (mode == "r")
      ctx.stdin_behavior = bp::close_stream();
    else if (mode == "w")
      ctx.stdout_behavior = bp::close_stream();
    else if (mode != "" && mode != "r+")
      throw exception("Subprocess: invalid popen mode '" + mode + "'");
  }

  optional<std::string> get_string( object const &o, char const* prop ) {
    if( !o.has_property(prop) )
      return boost::none;

    value v = o.get_property(prop);
    if ( v.is_string() )
      throw exception( format("subprocess.popen: optional property `%s' is not a string") % prop, "TypeError");
    return v.to_std_string();
  }

  tribool get_bool( object const &o, char const* prop ) {
    if( !o.has_property(prop) )
      return indeterminate;

    value v = o.get_property(prop);
    return v.to_boolean();
  }

  // Validate the object arg passed to subprocess.popen has everything it needs.
  value popen_from_obj(object &o, bp::context &ctx) {

    optional<std::string> exe = get_string( o, "executable" );

    value v = o.get_property("args");
    if (!v.is_object() && v.get_object().is_array()) {
      throw exception( "subprocess.popen: required property `args' is not an array", "TypeError");
    }
    std::vector<std::string> args = array_to_vector( array( v.get_object() ) );

    if (!exe)
      exe = args.front();

    tribool stdin_  = get_bool( o, "stdin" ),
            stdout_ = get_bool( o, "stdout" ),
            stderr_ = get_bool( o, "stderr" );

    // Should these be close or silence?
    if (!stdin_)
      ctx.stdin_behavior = bp::close_stream();
    if (!stdout_)
      ctx.stdout_behavior = bp::close_stream();
    if (!stderr_)
      ctx.stderr_behavior = bp::close_stream();

    if ( get_bool( o, "shell") ) {
#if defined(BOOST_POSIX_API)

      exe = "/bin/sh";
      args.insert( args.begin(), 2, "");
      args[0] = "sh";
      args[1] = "-c";
#elif defined(BOOST_WINDOWS_API)
      char sysdir[MAX_PATH];
      UINT size = ::GetSystemDirectoryA(sysdir, sizeof(sysdir));
      if (!size) {
        boost::throw_exception(
          bs::system_error(bs::error_code(::GetLastError(), bs::get_system_category()),
            "subprocess.popen: GetWindowsDirectory failed"
          )
        );
      }
      BOOST_ASSERT(size < MAX_PATH);

      exe = std::string(sysdir) + (sysdir[size - 1] != '\\' ? "\\cmd.exe" : "cmd.exe");
      args.insert( args.begin(), 2, "");
      args[0] = "cmd";
      args[1] = "/c";
#endif
    }

    if (!o.has_own_property("env"))
      ctx.environment = bp::self::get_environment();
    else if (o.get_property("env").is_object() == false)
      throw exception("subprocess.popen: 'env' property must be an object","TypeError");
    else {
      bp::environment env;
      object const &js_env = o.get_property("env").to_object();
      for ( property_iterator i = js_env.begin(), end = js_env.end();
            i != end; ++i )
      {
        if ( !js_env.has_own_property(*i) )
          continue;
        env.insert(boost::process::environment::value_type(
          i->to_std_string(), js_env.get_property(*i).to_std_string()
        ));
      }
      ctx.environment = env;
    }

    bp::child c = bp::launch( exe.get(), args, ctx );
    return create<subprocess::Subprocess>( boost::fusion::make_vector( c, ctx ) );
  }
}

void subprocess::popen(flusspferd::call_context &x) {
  bp::context ctx;

  // Default stream behaviours:
  ctx.stdin_behavior = bp::capture_stream();
  ctx.stdout_behavior = bp::capture_stream();
  ctx.stderr_behavior = bp::capture_stream();

  size_t n = x.arg.size();

  if (n < 1)
    throw exception("subprocess.popen: not enough parameters");

  // Possible arg forms:
  //  popen( "command" )
  //  popen( "command", "mode" )
  //  popen( ["command"] )
  //  popen( ["command"], "mode" )
  //  popen( { ... } )

  std::string mode;
  if ( n >= 2 && !x.arg[0].is_undefined_or_null() ) {
    // Make sure the arg is a string, and store it back there so its rooted.
    string s = x.arg[1].to_string();
    x.arg[1] = s;
    mode = s.to_string();
  }


  value v = x.arg[0];
  if ( v.is_object() ) {
    object o = v.get_object();

    if ( o.is_array() ) {
      //  popen( ["command"] )
      //  popen( ["command"], "mode" )
      std::vector<std::string> args = array_to_vector( array( o ) );

      // Copy the current environment
      ctx.environment = bp::self::get_environment();

      bp::child c = bp::launch( args.front(), args, ctx );

      apply_mode_string( ctx, mode );
      x.result = create<Subprocess>( boost::fusion::make_vector( c, ctx ) );
    }
    else {
      //  popen( { ... } )

      x.result = popen_from_obj( o, ctx );
    }
  }
  else {
    //  popen( "command" )
    //  popen( "command", "mode" )

    apply_mode_string( ctx, mode );

    // Copy the current environment
    ctx.environment = bp::self::get_environment();

    bp::child child = bp::launch_shell(v.to_std_string(), ctx );
    x.result = create<Subprocess>( boost::fusion::make_vector( child, ctx ) );
  }

}

