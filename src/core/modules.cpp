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
#include "flusspferd/create/object.hpp"
#include "flusspferd/create/array.hpp"
#include "flusspferd/io/file.hpp"
#include "flusspferd/io/filesystem-base.hpp"
#include "flusspferd/binary.hpp"
#include "flusspferd/encodings.hpp"
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#else
#define SHLIBPREFIX "lib"
#include <dlfcn.h>
#endif


using namespace flusspferd;
using namespace flusspferd::param;

namespace algo = boost::algorithm;
namespace fs = boost::filesystem;
namespace fusion = boost::fusion;

static fs::path make_dsoname(std::string const &id);

// Create |require| function on container.
void flusspferd::load_require_function(object container) {
  container.set_property("require", require::create_require());
}


require::require()
  : native_function_base(1, "require"),
    module_cache(flusspferd::create<object>()),
    paths(flusspferd::create<array>()),
    alias(flusspferd::create<object>()),
    preload(flusspferd::create<object>()),
    main(flusspferd::create<object>())
{ }

// Copy constructor. Keep the same JS objects for the state variables
require::require(require const &rhs)
  : native_function_base(1, "require"),
    module_cache(rhs.module_cache),
    paths(rhs.paths),
    alias(rhs.alias),
    preload(rhs.preload),
    main(rhs.main)
{ }

require::~require() {}

// Static helper method to actually create |require| function objects
object require::create_require() {
  local_root_scope scope;

  object fn = create_native_functor_function<require>(object());
  require* r = static_cast<require*>(native_function_base::get_native(fn));

  const property_flag perm_ro = permanent_property | read_only_property;

  fn.define_property("module_cache", r->module_cache, perm_ro);
  fn.define_property("paths", r->paths, perm_ro);
  fn.define_property("alias", r->alias, perm_ro);
  fn.define_property("preload", r->preload, perm_ro);
  fn.define_property("main", r->main, perm_ro);
  return fn;
}

// Each module wants a different |require| object, so that it can have a
// different require.id property
object require::new_require_function(string const &id) {
  // Use the copy ctor form to share the JS state variables.
  object new_req = create_native_functor_function<require>(object(), *this);
  new_req.set_prototype(*this);

  new_req.define_property("id", id, permanent_property|read_only_property);

  return new_req;
}

void require::set_main_module(std::string const &id_) {
  id_classification type = classify_id(id_);

  if (type == relative)
    throw exception("require.main cannot be set using a relative id", "TypeError");

  fs::path mod;
  if (type == top_level) {
    mod = find_top_level_js_module(id_, true).get_value_or("ARGH");
  }
  else {
    mod = io::fs_base::canonicalize( id_.substr(strlen("file://")) );
  }

  std::string id = "file://" + mod.string();
  main.set_property("id", id);
  main.set_property("uri", id);
}

// The implementation of the |require()| function that is available to JS
void require::call(call_context &x) {
  std::string id = x.arg[0].to_std_string();

  // If what ever they require is already loaded, give it to them
  if (module_cache.has_own_property(id)) {
    x.result = module_cache.get_property(id);
    return;
  }

  id_classification type = classify_id(id);

  if (type == top_level) {
    x.result = load_top_level_module(id);
    return;
  }

  fs::path module_path;
  if (type == relative) {
    module_path = resolve_relative_id( id );
    id = module_path.string();
  }
  else if (type == fully_qualified) {
    id = id.substr(strlen("file://"));
    module_path = io::fs_base::canonicalize( id );
  }
  id = "file://" + id;


  // If what ever the file resolves to is already loaded, give it to them
  if (module_cache.has_own_property(id)) {
    x.result = module_cache.get_property(id);
    return;
  }

  x.result = load_absolute_js_file(module_path, id);
}


string require::load_module_text(fs::path filename) {
  io::file &f = create<io::file>(
    fusion::make_vector(filename.string().c_str(), value("r")));

  // buffer blob
  byte_array &blob = create<byte_array>(
    fusion::vector2<binary::element_type*, std::size_t>(0, 0));
  binary::vector_type &buf = blob.get_data();

  // Look for a shebang line
  f.read_binary(2, blob);

  if (buf[0] == '#' && buf[1] == '!') {
    // Shebang line - skip the line, but insert an empty one in there to keep
    // source line numbers right
    buf.clear();
    buf.push_back('\n');
    f.read_line(value("\n"));
  }
  f.read_whole_binary(blob);

  // TODO: Some way of supporting other encodings is probably useful
  return encodings::convert_to_string("UTF-8", blob);

}

/// Load the given @c filename as a module
void require::require_js(fs::path filename, std::string const &id, object exports) {
  class StrictModeScopeGuard {
      bool old_strict;
    public:
      StrictModeScopeGuard(bool v) : old_strict(v) {}

      ~StrictModeScopeGuard() {
        flusspferd::current_context().set_strict(old_strict);
      }
  };
  // Reset the strict mode when we leave (the REPL might have it off)
  StrictModeScopeGuard guard(flusspferd::current_context().set_strict(true));

  local_root_scope root_scope;

  string module_text = load_module_text(filename);

  std::vector<std::string> argnames;
  argnames.push_back("exports");
  argnames.push_back("require");
  argnames.push_back("module");

  std::string fname = filename.string();
  function fn = flusspferd::create<flusspferd::function>(
      _name = fname,
      _argument_names = argnames,
      _source = module_text,
      _file = fname.c_str(),
      _line = 1ul);

  object module;

  // Are we requring the main module?
  if (main.get_property("id")== id) {
    module = main;
  }
  else {
    module = create<object>();
    module.set_property("uri", id);
    module.set_property("id", id);
  }

  object require = new_require_function(id);

  fn.call(fn, exports, require, module);
}

/// What type of require id is @c id
require::id_classification require::classify_id(std::string const &id) {
  if (algo::starts_with(id, "./") || algo::starts_with(id, "../"))
    return relative;
  if (algo::starts_with(id, "file://"))
    return fully_qualified;
  return top_level;
}

/**
 * Resolve a realtive ID (as passed to require) using the current module id
 * returning a canonical filename
 *
 * @param id The require id to resolve into an absolute path
 * @return boost::filesystem::path object
 */
fs::path require::resolve_relative_id(std::string const &id) {

  fs::path module(current_id().substr(strlen("file://")));
  module = module.parent_path();

  return io::fs_base::canonicalize( module / (id +".js") );
}


// Utility class to remove |module_cache[id]| in case of an exception
class ExportsScopeGuard {
    object module_cache;
    std::string id;
  public:
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



object load_native_module(fs::path const &dso_name, object exports) {
  std::string const &fullpath = dso_name.string();
#ifdef WIN32
  HMODULE module = LoadLibrary(fullpath.c_str());

  // TODO: Imrpove error message
  if (!module)
    throw exception(("Unable to load library '" +fullpath+"'"));

  FARPROC symbol = GetProcAddress(module, "flusspferd_load");

  if (!symbol)
    throw exception(("Unable to load library '" + fullpath + "': symbol "
                    "not found"));
#else
  // Load the .so
  void *module = dlopen(fullpath.c_str(), RTLD_LAZY);
  if (!module) {
    std::stringstream ss;
    ss << "Unable to load library '" << fullpath
       << "': " << dlerror();
    throw exception(ss.str());
  }

  dlerror(); // clear error state

  void *symbol = dlsym(module, "flusspferd_load");

  char const *const error_string = dlerror();

    if (error_string) {
      std::stringstream ss;
      ss << "Unable to load library '" << fullpath
         << "': " << error_string;
      throw exception(ss.str());
    }
#endif

  flusspferd_load_t func = *(flusspferd_load_t*) &symbol;

  object context = global();
  func(exports, context);

  return exports;
}

std::string require::current_id() {
  return get_property("id").to_std_string();
}


// Loading of top-level IDs is more complex then relative or abs uris
// We need to check alias and prelaod, and also search the require paths for
// .js files and DSOs
object require::load_top_level_module(std::string &id) {

  if (alias.has_own_property(id)) {
    std::string new_id = alias.get_property(id).to_std_string();

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

  object classes_object = flusspferd::global();
  object ctx = flusspferd::create<object>(classes_object);
  ctx.set_parent(classes_object);

  root_object exports(flusspferd::create<object>());

  ctx.define_property(
    "exports",
    exports,
    read_only_property | permanent_property);

  ExportsScopeGuard scope_guard(module_cache, id);
  module_cache.set_property(id, exports);

  if (!preload.is_null()) {
    // Check for 'preloaded' module
    value loader = preload.get_property(id);
    if (loader.is_object() && !loader.is_null()) {
      object o = loader.get_object();
      o.call(ctx);
      scope_guard.exit_cleanly();

      return exports;
    }
  }

  size_t len = paths.length();
  bool found = false;

  fs::path dso_name = make_dsoname(id);

  for (size_t i = 0; i < len; i++) {
    fs::path path = io::fs_base::canonicalize(paths.get_element(i).to_std_string());
    fs::path native_path = path / dso_name;
    if (sec.check_path(native_path.string(), security::READ) &&
        fs::exists(native_path) )
    {
      found = true;
      load_native_module(native_path, exports);
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
      exports = module_cache.get_property_object(new_id);
    }
    else {
      // Cache it under the top-level and fully-qualified ids
      ExportsScopeGuard scope_guard2(module_cache, new_id);
      module_cache.set_property(new_id, exports);

      require_js(js_name.get(), new_id, exports);
      scope_guard2.exit_cleanly();
    }
  }

  scope_guard.exit_cleanly();

  return exports;
}

boost::optional<fs::path>
require::find_top_level_js_module(std::string const &id, bool fatal) {
  fs::path js_name = fs::path(id + ".js");

  size_t len = paths.length();

  for (size_t i = 0; i < len; i++) {
    fs::path path = io::fs_base::canonicalize(paths.get_element(i).to_std_string());

    fs::path js_path = path / js_name;

    // Check if we loaded something by this name previously, even if the file
    // doesn't exist anymore
    std::string new_id = "file://" + js_path.string();
    if (module_cache.has_own_property(new_id)) {
      return js_path;
    }

    if ( fs::exists(js_path) )
      return js_path;
  }

  if (fatal) {
    std::stringstream ss;
    ss << "Unable to find library '" << id << "' in [" << paths << "]";
    throw exception(ss.str());
  }
  return boost::none;
}

object require::load_absolute_js_file(fs::path path, std::string &id) {
  security &sec = security::get();

  ExportsScopeGuard scope_guard(module_cache, id);
  if (sec.check_path(path.string(), security::READ) &&
      fs::exists(path))
  {
    root_object exports(flusspferd::create<object>());

    module_cache.set_property(id, exports);
    require_js(path, id, exports);
    scope_guard.exit_cleanly();
    return exports;
  }

  std::stringstream ss;
  ss << "Unable to load library '" << id;
  throw exception(ss.str());
}


static fs::path make_dsoname(std::string const &id) {
  fs::path p(id);

  p.replace_extension(FLUSSPFERD_MODULE_SUFFIX);
#ifdef SHLIBPREFIX
  std::string file = SHLIBPREFIX + p.filename();
  p = p.remove_filename() / file;
#endif

  return p;
}

