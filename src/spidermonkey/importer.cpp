// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#include "flusspferd/importer.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/security.hpp"
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <errno.h>

#include <dlfcn.h>

#define DIRSEP1 "/"
#define DIRSEP2 ""
#define SHLIBPREFIX "lib"

#ifdef APPLE
#define SHLIBSUFFIX ".dylib"
#else
#define SHLIBSUFFIX ".so"
#endif

namespace flusspferd {

void import(call_context &);

void load_import_function(object container) {
  function imp = create_native_function(container, "Import", &import, 2);
  container.define_property("import", imp, object::dont_enumerate);

  imp.define_property("preload", create_object(), object::permanent_property);
  imp.define_property("paths", create_array(), object::permanent_property);
  imp.define_property("alias", create_object(), object::permanent_property);
  imp.define_property("module_cache", create_object(),
                      object::permanent_property);
}

}

using namespace flusspferd;

namespace {

// Take 'foo.bar' as a flusspferd::string, check no path sep in it, and
// return '/foo/bar.js' or '/foo/libbar.so', etc. as a std::string
std::string process_name(std::string const &name, bool for_script) {
  std::string p = name;
  if (p.find(DIRSEP1, 0) != std::string::npos &&
      p.find(DIRSEP2, 0) != std::string::npos) {
    throw exception("Path seperator not allowed in module name");
  }

  std::size_t pos = 0;
  while ( (pos = p.find('.', pos)) != std::string::npos) {
    p.replace(pos, 1, DIRSEP1);
    pos++;
  }

  if (!for_script && SHLIBPREFIX) {
    // stick the lib on the front as needed
    pos = p.rfind(DIRSEP1, 0);
    if (pos == std::string::npos)
      pos = 0;
    p.insert(pos, SHLIBPREFIX);
  }

  p = DIRSEP1 + p;
  if (for_script)
    p += ".js";
  else
    p += SHLIBSUFFIX;

  return p;
}

}

void flusspferd::import(call_context &x) {
  security &sec = security::get();

  std::string name = flusspferd::string(x.arg[0]).to_string();
  bool binary_only = x.arg[1].to_boolean();

  value alias_v = x.function.get_property("alias");
  
  if (alias_v.is_object() && !alias_v.is_null()) {
    object alias = alias_v.get_object();
    if (alias.has_own_property(name))
      name = alias.get_property(name).to_string().to_string();
  }

  object module_cache = x.function.get_property("module_cache").to_object();
  if (module_cache.is_null())
    throw exception("No valid module cache");
 
  std::string key = name + (binary_only ? ";binary-only" : "");
  if (module_cache.has_own_property(key)) {
    x.result = module_cache.get_property(key);
    return;
  }

  object ctx = flusspferd::global();

  value preload = x.function.get_property("preload");

  if (preload.is_object() && !preload.is_null()) {
    value loader = preload.get_object().get_property(name);
    if (loader.is_object()) {
      if (!loader.is_null()) {
        local_root_scope scope;
        object o = loader.get_object();
        x.result = o.call(ctx);
      }
      return;
    }
  }

  std::string so_name, js_name;
  so_name = process_name(name, false);
  js_name = process_name(name, true);
  
  value paths_v = x.function.get_property("paths").to_object();
  if (!paths_v.is_object() || paths_v.is_null())
    throw exception("Unable to get search paths or it is not an object");

  array paths = paths_v.get_object();

  size_t len = paths.length();
  for (size_t i=0; i < len; i++) {
    std::string path = paths.get_element(i).to_string().to_string();
    std::string fullpath = path + js_name;

    if (!binary_only)
      if (sec.check_path(fullpath, security::READ))
        if (boost::filesystem::exists(fullpath)) {
          value val = current_context().execute(
              fullpath.c_str(), ctx);
          module_cache.set_property(key, val);
          x.result = val;
          return;
        }

    fullpath = path + so_name;

    if (!sec.check_path(fullpath, security::READ))
      continue;

    if (boost::filesystem::exists(fullpath)) {
      // Load the .so
      void *module = dlopen(fullpath.c_str(), RTLD_LAZY);
      if (!module) {
        std::stringstream ss;
        ss << "Unable to load library '" << fullpath.c_str()
           << "': " << dlerror();
        throw exception(ss.str().c_str());
      }

      void *symbol = dlsym(module, "flusspferd_load");

      if (!symbol) {
        std::stringstream ss;
        ss << "Unable to load library '" << fullpath.c_str() 
           << "': " << dlerror();
        throw exception(ss.str().c_str());
      }

      flusspferd_load_t func = *(flusspferd_load_t*) &symbol;

      value val = func(ctx);
      module_cache.set_property(key, val);
      x.result = val;
      return;
    }
  } 

  std::stringstream ss;
  ss << "Unable to find library '";
  ss << name.c_str();
  ss << "' in [";
  ss << paths_v.to_string().c_str() << "]";
  throw exception(ss.str().c_str());
}

