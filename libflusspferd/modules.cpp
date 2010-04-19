// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
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

#include "flusspferd/modules.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/value_io.hpp"
#include "flusspferd/create_on.hpp"
#include "flusspferd/create/array.hpp"
#include "flusspferd/create/object.hpp"
#include "flusspferd/io/file.hpp"
#include "flusspferd/io/filesystem-base.hpp"
#include "flusspferd/binary.hpp"
#include "flusspferd/encodings.hpp"
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/scope_exit.hpp>
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#else
#define SHLIBPREFIX "lib"
#include <dlfcn.h>
#endif


using namespace flusspferd;
using namespace flusspferd::param;

namespace phoenix = boost::phoenix;
namespace args = phoenix::arg_names;
namespace algo = boost::algorithm;
namespace fs = boost::filesystem;
namespace fusion = boost::fusion;
using boost::format;

namespace {
static fs::path make_dsoname(std::string const &id);
static void load_native_module(fs::path const &dso_name, object exports);
static void setup_module_resolve_fns(object &module, fs::path const &id);

static std::string module_resource_resolve(fs::path const &module_dir, std::string const &x);
static object module_resource(const fs::path &module_dir, std::string const &x_, value mode);

// This function based largley on one from boost/program_options. See end of file for its license
static array split_args_string(const value& input);

static const format load_error_fmt("Unable to load module '%1%': %2%");
}

// Create |require| function on container.
void flusspferd::load_require_function(object container) {
  container.set_property("require", require::create_require());
}


require::require(object const &fun)
  : native_function_base(fun)
{ }

// Copy constructor. Keep the same JS objects for the state variables
require::require(object const &fun, require const &rhs)
  : native_function_base(fun),
    module_cache(rhs.module_cache),
    paths(rhs.paths),
    alias(rhs.alias),
    preload(rhs.preload),
    main(rhs.main)
{ }

require::~require() {}

// Static helper method to actually create |require| function objects
object require::create_require() {
  root_object fn(create<require>(param::_name = "require"));
  require* r = static_cast<require*>(native_function_base::get_native(fn));

  const property_flag perm_ro = permanent_property | read_only_property;

  root_object module_cache(create<object>());
  r->module_cache = module_cache;

  root_array paths(create<array>());
  r->paths = paths;

  root_object alias(create<object>());
  r->alias = alias;

  root_object preload(create<object>());
  r->preload = preload;

  root_object main(create<object>());
  r->main = main;

  fn.define_property("module_cache", module_cache, perm_ro);
  fn.define_property("paths", paths, perm_ro);
  fn.define_property("alias", alias, perm_ro);
  fn.define_property("preload", preload, perm_ro);
  fn.define_property("main", main, perm_ro);

  return fn;
}

// Each module wants a different |require| object, so that it can have a
// different require.id property
object require::new_require_function(string const &id_) {
  root_string id(id_);

  // Use the copy ctor form to share the JS state variables.
  root_object new_req(create<require>(
                          fusion::vector1<require&>(*this)
                          , param::_name = "require"
                      ));
  new_req.set_prototype(*this);

  new_req.define_property("id", id, permanent_property|read_only_property);

  return new_req;
}

void require::set_main_module(std::string const &id_) {
  id_classification const type = classify_id(id_);

  if (type == relative)
    throw exception("require.main cannot be set using a relative id", "TypeError");

  fs::path mod;
  if (type == top_level) {
    boost::optional<fs::path> mod_ = find_top_level_js_module(id_, true);

    if (!mod_) {
      throw exception(format("unable to find top level module when setting require.main: \"%s\"") % id_);
    }
    mod = *mod_;
  }
  else {
    mod = io::fs_base::canonicalize( id_.substr(sizeof("file://")-1) );
  }

  std::string id = "file://" + mod.string();
  main.set_property("id", id);
  main.set_property("uri", id);
  setup_module_resolve_fns(main, mod);
  set_property("id", id);
}

// require.id
std::string require::current_id() {
  return get_property("id").to_std_string();
}

// The implementation of the |require()| function that is available to JS
void require::call(call_context &x) {

  std::string id = x.arg[0].to_std_string();
  x.result = call_helper(id).get_property("exports");

  gc(true); // maybe-gc
}

// Helper method that returns the cache object. Doing this makes various code
// paths a lot easier
object require::call_helper(std::string const &id_) {

  // If what ever they require is already loaded, give it to them
  if (module_cache.has_own_property(id_)) {
    return module_cache.get_property_object(id_);
  }

  switch (classify_id(id_)) {
    default:
    case top_level:
      return load_top_level_module(id_);
    case relative:
      return load_relative_module(id_);
    case fully_qualified:
      return load_absolute_module(id_);
  }

}


string require::load_module_text(fs::path filename, boost::optional<object> opts) {
  root_string read_only("r");

  io::file &f = create<io::file>(
    fusion::vector2<char const*, string>(filename.string().c_str(), read_only));
  root_object f_o(f);

  // buffer blob
  byte_array &blob = create<byte_array>(
    fusion::vector2<binary::element_type*, std::size_t>(0, 0));
  root_object b_o(blob);
  binary::vector_type &buf = blob.get_data();

  // Look for a shebang line
  f.read_binary(2, blob);

  binary::vector_type::iterator i, s;

  if (buf[0] == '#' && buf[1] == '!') {
    // Shebang line - skip the line, but insert a comment line here to keep
    // source line numbers right
    buf.clear();
    buf.push_back('/');
    buf.push_back('/');
  }
  f.read_whole_binary(blob);


  // Look for coding and option lines. An coding line looks like one of
  // "// -*- coding:utf-8 -*-"
  // "// vim:fileencoding=utf-8:"
  //
  // An option line looks like
  // "// flusspferd: -xboo"
  //
  // We continue looking until we see a blank comment or a non comment line

  using namespace boost::xpressive;
  sregex opt_re = sregex::compile("^\\s*([-\\w.]+):\\s*(.*)$");
  sregex coding_re = sregex::compile("^.*coding[:=]\\s*([-\\w.]+)");
  sregex empty_line_re = bos >> *_s >> eos;

  // We only want to look for a coding comment on line 1 or 2
  int look_for_coding = 2;
  std::string encoding = "UTF-8";

  for (i = buf.begin(); i != buf.end(); ++i) {
    if (*(i++) != '/' || *(i++) != '/') {
      // Not a comment line - stop!
      break;
    }
    binary::vector_type::iterator e;
    e = std::find(i, buf.end(), '\n');
    if (e == buf.end())
      break;

    std::string line( reinterpret_cast<char const *>(&*i), size_t(e-i) );

    // Move onto next line
    i = e;

    smatch m;
    if (look_for_coding-- && regex_match(line, m, coding_re)) {
      // Huzzah! We have an encoding!
      encoding = m[1];
      look_for_coding = 0;

      continue;
    }

    // Empty comment line - stop looking
    if (regex_match(line, empty_line_re))
      break;

    if (opts && regex_match(line, m, opt_re)) {
      // A line we are interested in, and we have somewhere to store the result
      opts->set_property(m[1].str(), m[2].str());
    }
  }

  // If we have "flusspferd" or "warnings" in the option, split them on
  // whitespace like shells do. TODO: Should we just split everything?
  if (opts) {
    value v = opts->get_property("flusspferd");
    if (v.is_string()) {
      opts->set_property( "flusspferd", split_args_string(v) );
    }

    v = opts->get_property("warnings");
    if (v.is_string()) {
      opts->set_property( "warnings", split_args_string(v) );
    }
  }

  return encodings::convert_to_string(encoding, blob);

}

/// Load the given @c filename as a module
void require::require_js(fs::path filename, std::string const &id, object cache) {
  bool const old_strict = flusspferd::current_context().set_strict(true);
  BOOST_SCOPE_EXIT((old_strict)) {
    // Reset the strict mode when we leave (the REPL might have it off)
    flusspferd::current_context().set_strict(old_strict);
  } BOOST_SCOPE_EXIT_END;

  root_string module_text(load_module_text(filename, cache.get_property_object("options")));

  std::vector<std::string> argnames;
  argnames.push_back("exports");
  argnames.push_back("require");
  argnames.push_back("module");

  std::string fname = filename.string();
  root_object fn(create<function>(
      _name = fname,
      _argument_names = argnames,
      _function = module_text,
      _file = fname.c_str(),
      _line = 1ul));

  root_object module;

  // Are we requring the main module?
  if (main.get_property("id") == id) {
    module = main;
  }
  else {
    module = create<object>();
    module.set_property("uri", id);
    module.set_property("id", id);
    setup_module_resolve_fns(module, filename);
  }

  root_object require(new_require_function(id));

  fn.call(fn, cache.get_property("exports"), require, module);
}

/// What type of require id is @c id
require::id_classification require::classify_id(std::string const &id) {
  if (algo::starts_with(id, "./") || algo::starts_with(id, "../"))
    return relative;
  if (algo::starts_with(id, "file://"))
    return fully_qualified;
  return top_level;
}


// Utility class to remove |module_cache[id]| in case of an exception
class ExportsScopeGuard {
  public:
    object module_cache;
    std::string id;

    ExportsScopeGuard(object _cache, std::string _id)
      : module_cache(_cache),
        id(_id)
    {}

    ~ExportsScopeGuard() {
      if (!module_cache.is_null())
        module_cache.delete_property(id);
    }

    void exit_cleanly() {
      // Replace object with null
      module_cache = object();
    }
};

object require::load_relative_module(std::string id) {

  fs::path module(current_id().substr(sizeof("file://")-1));

  module = io::fs_base::canonicalize( module.parent_path() / (id + ".js") );
  id = module.string();
  fs::path dso_path = make_dsoname(module.string());
  id = "file://" + id;
  std::string const dso_id = "file://" + dso_path.string();


  // If either of the JS or DSO is already cached then just return it
  if (module_cache.has_own_property(id) )
    return module_cache.get_property_object(id);
  else if (module_cache.has_own_property(dso_id) )
    return module_cache.get_property_object(dso_id);

  bool js = false, dso = false;
  security &sec = security::get();
  if (sec.check_path(module.string(), security::READ) && fs::exists(module)) {
    js = true;
  }
  if (sec.check_path(dso_path.string(), security::READ) && fs::exists(dso_path)) {
    dso = true;
  }

  if (!js && !dso)
    throw exception(format(load_error_fmt) % id % "file not found");
  else if (!js)
    id = dso_id;


  ExportsScopeGuard scope_guard(module_cache, id);

  // The object we store in module_cache
  object cache = create_cache_entry(id);

  if (dso)
    load_native_module(dso_path, cache.get_property_object("exports"));
  if (js)
    require_js(module, id, cache);

  scope_guard.exit_cleanly();

  return cache;
}

namespace {
static void load_native_module(fs::path const &dso_name, object exports) {
  std::string const &fullpath = dso_name.string();
#ifdef WIN32
  HMODULE module = LoadLibrary(fullpath.c_str());

  // TODO: Improve error message
  if (!module)
    throw exception(format(load_error_fmt) % fullpath % "can't open DLL");

  FARPROC symbol = GetProcAddress(module, "flusspferd_load");

  if (!symbol)
    throw exception(format(load_error_fmt) % fullpath % "symbol not found");
#else
  // Load the .so
  void *module = dlopen(fullpath.c_str(), RTLD_LAZY);
  if (!module) {
    throw exception(format(load_error_fmt) % fullpath % dlerror());
  }

  dlerror(); // clear error state

  void *symbol = dlsym(module, "flusspferd_load");

  char const *const error_string = dlerror();

  if (error_string) {
    throw exception(format(load_error_fmt) % fullpath % error_string);
  }
#endif

  flusspferd_load_t func = *(flusspferd_load_t*) &symbol;

  root_object context(global());
  func(exports, context);
}
}

// Loading of top-level IDs is more complex then relative or abs uris
// We need to check alias and prelaod, and also search the require paths for
// .js files and DSOs
object require::load_top_level_module(std::string const &id) {
  root_array paths(this->paths);

  if (alias.has_own_property(id)) {
    std::string const new_id = alias.get_property(id).to_std_string();

    // Sanity check - aliased ID should be toplevel too
    if (classify_id(new_id) != top_level) {
      throw exception("'" + id + "' must be aliased to a top-level id - not '" +
                      new_id);
    }

    object e = load_top_level_module(new_id);
    // Cache the aliased version under both ids
    module_cache.set_property(id, e);
    return e;
  }

  security &sec = security::get();

  ExportsScopeGuard scope_guard(module_cache, id);

  // The object we store in module_cache
  object cache = create_cache_entry(id);

  if (!preload.is_null()) {
    // Check for 'preloaded' module
    value loader = preload.get_property(id);
    if (loader.is_object() && !loader.is_null()) {
      object o = loader.get_object();

      root_object classes_object(flusspferd::global());
      root_object ctx(flusspferd::create<object>(classes_object));
      ctx.set_parent(classes_object);
      ctx.set_property("exports", cache.get_property("exports"));

      o.call(ctx);
      scope_guard.exit_cleanly();

      return cache;
    }
  }

  std::size_t const len = paths.length();
  bool found = false;

  fs::path dso_name = make_dsoname(id);

  for (size_t i = 0; i < len; i++) {
    fs::path path = paths.get_element(i).to_std_string();
    fs::path native_path = io::fs_base::canonicalize( path / dso_name );

    std::string new_id = "file://" + native_path.string();
    if (module_cache.has_own_property(new_id)) {
      // This dso is already cached.
      cache = module_cache.get_property_object(new_id);
      // Cache it under the top level name
      module_cache.set_property(id, cache);
      // Short-circuit and return it
      scope_guard.exit_cleanly();
      return cache;
    }
    else if (sec.check_path(native_path.string(), security::READ) &&
        fs::exists(native_path) )
    {
      found = true;
      dso_name = native_path;
      load_native_module(native_path, cache.get_property_object("exports"));
      break;
    }
  }

  boost::optional<fs::path> js_name = find_top_level_js_module(id, !found);

  if (js_name) {
    found = true;
    std::string new_id = "file://" + js_name->string();
    // Check if we loaded something by this name previously, even if the file
    // doesn't exist anymore
    if (module_cache.has_own_property(new_id)) {
      cache = module_cache.get_property_object(new_id);
      // And cache it under the top level name
      module_cache.set_property(id, cache);
    }
    else {
      // Cache it under the top-level and fully-qualified ids
      ExportsScopeGuard scope_guard2(module_cache, new_id);
      module_cache.set_property(new_id, cache);

      require_js(js_name.get(), new_id, cache);
      scope_guard2.exit_cleanly();
    }
  }
  else {
    // We loaded just a dso, cache that under its full name too.
    module_cache.set_property("file://" + dso_name.string(), cache);
  }

  scope_guard.exit_cleanly();

  return cache;
}

boost::optional<fs::path>
require::find_top_level_js_module(std::string const &id, bool fatal) {
  root_array paths(this->paths);

  fs::path js_name = fs::path(id + ".js");

  std::size_t const len = paths.length();

  for (size_t i = 0; i < len; i++) {
    fs::path path = paths.get_element(i).to_std_string();

    fs::path js_path = io::fs_base::canonicalize( path / js_name );

    // Check if we loaded something by this name previously, even if the file
    // doesn't exist anymore
    std::string const new_id = "file://" + js_path.string();
    if (module_cache.has_own_property(new_id)) {
      return js_path;
    }

    if ( fs::exists(js_path) )
      return js_path;
  }

  if (fatal) {
    throw exception(
      format(load_error_fmt)
          % id % ("can't locate it in [" + value(paths).to_std_string() + "]")
    );
  }
  return boost::none;
}

object require::create_cache_entry(std::string const &id) {

  // module_cache[id] = {};
  object cache = create<object>(
    _name = id,
    _container = module_cache,
    _attributes = no_property_flag
  );

  // module_cache[id].exports = ...
  object exports = create<object>(
    _name = "exports",
    _container = cache,
    _attributes = read_only_property | permanent_property
  );

  create_on(exports)
    .create<function>(
      "toString",
      boost::phoenix::val("[module " + id + "]"),
      _signature = param::type<std::string ()>() )
    .create<function>(
      "toSource",
      boost::phoenix::val("(require('" + id + "'))"),
      _signature = flusspferd::param::type<std::string ()>() );

  // module_cache[id].options
  create<array>(
    _name = "options",
    _container = cache
  );

  return cache;
}

object require::load_absolute_module(std::string id) {
  security &sec = security::get();

  id = id.substr(sizeof("file://")-1);
  fs::path path = io::fs_base::canonicalize( id );
  id = "file://" + id;

  // If what ever the file resolves to is already loaded, give it to them
  if (module_cache.has_own_property(id)) {
    return module_cache.get_property_object(id);
  }

  ExportsScopeGuard scope_guard(module_cache, id);
  if (sec.check_path(path.string(), security::READ) &&
      fs::exists(path))
  {
    object cache = create_cache_entry(id);

    if (path.extension() == FLUSSPFERD_MODULE_SUFFIX)
      load_native_module(path, cache.get_property_object("exports"));
    else
      require_js(path, id, cache);
    scope_guard.exit_cleanly();
    return cache;
  }

  throw exception(format(load_error_fmt) % id % "file not found");
}

namespace {
static fs::path make_dsoname(std::string const &id) {
  fs::path p(id);

  p.replace_extension(FLUSSPFERD_MODULE_SUFFIX);
#ifdef SHLIBPREFIX
  std::string file = SHLIBPREFIX + p.filename();
  p = p.remove_filename() / file;
#endif

  return p;
}


static void setup_module_resolve_fns(object &module, fs::path const &path) {

  fs::path module_dir = fs::path(path).remove_filename();

  // Create the module.resource function. This isn't yet a CJS proposal, but it
  // has been hinted at. And its *damn* useful.
  object mod_req = flusspferd::create<flusspferd::function>(
    _name = "resource",
    _container = module,
    _function = phoenix::bind(&module_resource, module_dir, args::arg1, args::arg2),
    _signature = param::type<object (std::string const &, value)>()
  );

  phoenix::bind(&module_resource_resolve, module_dir, args::arg1);
  flusspferd::create<flusspferd::function>(
    _name = "resolve",
    _container = mod_req,
    _function = phoenix::bind(&module_resource_resolve, module_dir, args::arg1),
    _signature = param::type<std::string (std::string const &)>()
  );

}

// owner: No such file or directory "foo/bar"
static const format resolve_error_fmt("module.resource.resolve: %1% \"%2%\"");

static std::string module_resource_resolve(const fs::path &module_dir, std::string const &x_) {
  fs::path x = x_;

  if (x.has_root_path() || x.has_root_name()) {
    throw exception(format(resolve_error_fmt) % "absolute paths are not allowed" % x_);
  }

  return io::fs_base::canonicalize(module_dir / x).string();
}

static object module_resource(const fs::path &module_dir, std::string const &x_, value mode) {

  std::string const &path = module_resource_resolve(module_dir, x_);

  return create<io::file>(fusion::make_vector(path.c_str(), mode));
}

// This function adapted to use js array by Ash
//
// Original from :
//   http://svn.boost.org/svn/boost/trunk/libs/program_options/src/winmain.cpp
// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

static array split_args_string(value const &v) {
  std::string const &input = v.to_std_string();

  array result = create<array>();
  root_object root(result);

  std::string::const_iterator i = input.begin();
  std::string::const_iterator const e = input.end();
  for(;i != e; ++i) {
    if (!std::isspace(*i))
      break;
  }

  // Empty string.
  if (i == e)
    return result;

  std::string current;
  bool inside_quoted = false;
  unsigned backslash_count = 0;

  for(; i != e; ++i) {
    if (*i == '"') {
      // '"' preceded by even number (n) of backslashes generates
      // n/2 backslashes and is a quoted block delimiter
      if (backslash_count % 2 == 0) {
          current.append(backslash_count / 2, '\\');
          inside_quoted = !inside_quoted;
          // '"' preceded by odd number (n) of backslashes generates
          // (n-1)/2 backslashes and is literal quote.
      } else {
          current.append(backslash_count / 2, '\\');
          current += '"';
      }
      backslash_count = 0;
    } else if (*i == '\\') {
      ++backslash_count;
    } else {
      // Not quote or backslash. All accumulated backslashes should be
      // added
      if (backslash_count) {
          current.append(backslash_count, '\\');
          backslash_count = 0;
      }
      if (std::isspace(*i) && !inside_quoted) {
          // Space outside quoted section terminate the current argument
          result.push(current);
          current.resize(0);
          for(;i != e && std::isspace(*i); ++i)
              ;
          --i;
      } else {
          current += *i;
      }
    }
  }

  // If we have trailing backslashes, add them
  if (backslash_count)
    current.append(backslash_count, '\\');

  // If we have non-empty 'current' or we're still in quoted
  // section (even if 'current' is empty), add the last token.
  if (!current.empty() || inside_quoted)
    result.push(current);
  return result;
}

}
