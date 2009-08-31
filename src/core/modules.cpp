// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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
#include "flusspferd/string.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/value_io.hpp"
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/spirit/home/phoenix/core.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
#include <boost/utility.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#define DIRSEP1 '/'
#define DIRSEP2 '\0'
#define SHLIBPREFIX "lib"

using namespace flusspferd;

namespace algo = boost::algorithm;
namespace phoenix = boost::phoenix;
namespace args = phoenix::arg_names;

static void require(call_context &);

void flusspferd::load_require_function(object container) {
  function imp = create_native_function(container, "require", &require, 1);

  imp.define_property("preload", create_object(), permanent_property);
  imp.define_property("paths", create_array(), permanent_property);
  imp.define_property("alias", create_object(), permanent_property);
  imp.define_property("module_cache", create_object(),
                      permanent_property);
  imp.define_property("id", flusspferd::string(),
                      permanent_property);
}

// Take 'foo/bar' as a flusspferd::string, check no path sep in it, and
// return '/foo/bar.js' or '/foo/libbar.so', etc. as a std::string
static std::string process_name(
    std::string name,
    std::string module,
    std::string const &prefix,
    std::string const &suffix,
    char out_dirsep)
{
  if ((DIRSEP1 != '/' && name.find(DIRSEP1) != std::string::npos) &&
      (DIRSEP2 != '/' && DIRSEP2 && name.find(DIRSEP2) != std::string::npos))
  {
    throw exception("Invalid module name");
  }

  typedef std::list<std::string> container;

  module.erase(
    std::find(module.rbegin(), module.rend(), '/').base(),
    module.end());;

  if (algo::starts_with(name, "./") || algo::starts_with(name, "../"))
    name = module + "/" + name;

  container elements;
  algo::split(elements, name, args::arg1 == '/', algo::token_compress_on);

  if (elements.empty())
    throw exception("Invalid module name");

  elements.remove(".");

  for (container::iterator it = ++elements.begin(); it != elements.end();) {
    if (*it == ".." && it != elements.begin())
      it = elements.erase(boost::prior(it), boost::next(it));
    else
      ++it;
  }
  std::string result;

  container::iterator last = boost::prior(elements.end());

  for (container::iterator it = elements.begin(); it != last; ++it) {
    result.append(*it);
    result += out_dirsep;
  }

  result += prefix;
  result += *last;
  result += suffix;

  return result;
}

void require(call_context &x) {
  security &sec = security::get();

  value paths_v = x.function.get_property_object("paths");
  if (!paths_v.is_object() || paths_v.is_null())
    throw exception("Unable to get search paths or it is not an object");

  array paths = paths_v.get_object();
  size_t len = paths.length();

  bool found = false;

  std::string name = flusspferd::string(x.arg[0]).to_string();

  std::string module =
      x.function.get_property("id").to_std_string();

  std::string key = process_name(name, module, "", "", '/');

  object module_cache;

  try {
    x.function.set_property("id", flusspferd::string(key));

    value alias_v = x.function.get_property("alias");

    if (alias_v.is_object() && !alias_v.is_null()) {
      object alias = alias_v.get_object();
      if (alias.has_own_property(key)) {
        name = alias.get_property(key).to_std_string();
        key = process_name(name, "", "", "", '/');
      }
    }

    module_cache = x.function.get_property_object("module_cache");
    if (module_cache.is_null())
      throw exception("No valid module cache");

    if (module_cache.has_own_property(key)) {
      x.result = module_cache.get_property(key);
      return;
    }

    object classes_object = flusspferd::global().prototype();
    object ctx = flusspferd::create_object(classes_object);
    ctx.set_parent(classes_object);

    object exports = flusspferd::create_object();
    ctx.define_property(
      "exports",
      exports,
      read_only_property | permanent_property);

    module_cache.set_property(key, exports);
    x.result = exports;

    value preload = x.function.get_property("preload");

    if (preload.is_object() && !preload.is_null()) {
      value loader = preload.get_object().get_property(key);
      if (loader.is_object()) {
        if (!loader.is_null()) {
          local_root_scope scope;
          object o = loader.get_object();
          o.call(ctx);
        }
        return;
      }
    }

    std::string so_name, js_name;
    so_name = "/" + process_name(key, "", SHLIBPREFIX, FLUSSPFERD_MODULE_SUFFIX, DIRSEP1);
    js_name = "/" + process_name(key, "", "", ".js", DIRSEP1);

    for (size_t i = 0; i < len; i++) {
      std::string path = paths.get_element(i).to_std_string();
      std::string fullpath = path + so_name;

      if (sec.check_path(fullpath, security::READ) &&
          boost::filesystem::exists(fullpath))
      {
#ifdef WIN32
        HMODULE module = LoadLibrary(fullpath.c_str());

        if (!module)
          throw exception(("Unable to load library '" +fullpath+"'").c_str());

        FARPROC symbol = GetProcAddress(module, "flusspferd_load");

        if (!symbol)
          throw exception(("Unable to load library '" + fullpath + "': symbol "
                          "not found").c_str());
#else
        // Load the .so
        void *module = dlopen(fullpath.c_str(), RTLD_LAZY);
        if (!module) {
          std::stringstream ss;
          ss << "Unable to load library '" << fullpath.c_str()
             << "': " << dlerror();
          throw exception(ss.str().c_str());
        }

        dlerror(); // clear error state

        void *symbol = dlsym(module, "flusspferd_load");

        char const *const error_string = dlerror();

        if (error_string) {
          std::stringstream ss;
          ss << "Unable to load library '" << fullpath.c_str()
             << "': " << error_string;
          throw exception(ss.str().c_str());
        }
#endif

        flusspferd_load_t func = *(flusspferd_load_t*) &symbol;

        func(exports, ctx);

        // The exports reference might have been changed.
        module_cache.set_property(key, exports);
        x.result = exports;

        found = true;
        break;
      }
    }

    for (size_t i = 0; i < len; i++) {
      std::string path = paths.get_element(i).to_std_string();
      std::string fullpath = path + js_name;

      if (sec.check_path(fullpath, security::READ) &&
          boost::filesystem::exists(fullpath))
      {
        value val = flusspferd::execute(fullpath.c_str(), ctx);
        found = true;
        break;
      }
    }

    if (!found) {
      std::stringstream ss;
      ss << "Unable to find library '" << key << "' in [" << paths_v << "]";
      throw exception(ss.str().c_str());
    }
  } catch (...) {
    if (!module_cache.is_null())
      module_cache.delete_property(key);

    x.function.set_property("id", flusspferd::string(module));
    throw;
  }

  x.function.set_property("id", flusspferd::string(module));
}

