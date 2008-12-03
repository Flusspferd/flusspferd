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
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <errno.h>

// Wrap the windows calls to the *nix equivalents
#ifndef _WIN32
#include <dlfcn.h>

#define DIRSEP1 "/"
#define DIRSEP2 ""
#define SHLIBPREFIX "lib"

#ifdef APPLE
#define SHLIBSUFFIX ".dylib"
#else
#define SHLIBSUFFIX ".so"
#endif

#else

#include <windows.h>

#define DIRSEP1 "\\"
#define DIRSEP2 "/"
#define SHLIBPREFIX 0
#define SHLIBSUFFIX ".dll"

#define dlopen(x,y) (void*)LoadLibrary(x)
#define dlsym(x,y) (void*)GetProcAddress((HMODULE)x,y)
#define dlclose(x) FreeLibrary((HMODULE)x)

// FIXME - not thread safe?
const char *dlerror() {
  static char szMsgBuf[256];
  ::FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      ::GetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      szMsgBuf,
      sizeof szMsgBuf,
      NULL);
  return szMsgBuf;
}

#endif

using namespace flusspferd;

object importer::class_info::create_prototype() {
  object proto = create_object();
  return proto;
}

void importer::class_info::augment_constructor(object &ctor) {
  ctor.define_property("preload", create_object());

  ctor.define_property("defaultPaths", create_array(), read_only_property);

  create_native_function(ctor, "lockPaths", &importer::lock_paths);
}

void importer::lock_paths(object &ctor) {
  local_root_scope scope;
  ctor.define_property("pathsLocked", true, read_only_property);
  object paths = ctor.get_property("defaultPaths").to_object();
  paths.seal(false);
}

class importer::impl {
public:
  typedef std::pair<std::string, bool> key_type;
  typedef boost::unordered_map<key_type, value> module_cache_map;
  module_cache_map module_cache;
};

importer::importer(object const &obj, call_context &)
  : native_object_base(obj), p(new impl)
{
  local_root_scope scope;

  // Create the load method on the actual object itself, not on the prototype
  // That way the following works:
  // 
  // i.load('foo'); // Assume foo module defines this.foo = 'abc'
  // print(i.foo); // abc
  //
  // without the problem of load being overridden to do bad things
  add_native_method("load", 2);
  register_native_method("load", &importer::load);

  object constructor = get_constructor<importer>();

  // Store search paths
  array arr = constructor.get_property("defaultPaths").to_object();
  arr = arr.call("concat").to_object();
  set_property("paths", arr);
  if (constructor.get_property("pathsLocked").to_boolean()) {
    arr.seal(false);
  }

  // Create a context object, which is the object on which all modules are
  // evaluated
  object context = create_object();
  set_property("context", context);

  // this.contexnt.__proto__ = this.__proto__; 
  // Not sure we actually want to do this, but we can for now.
  context.set_prototype(get_prototype());
  set_prototype(context);
}

importer::~importer() {}

void importer::trace(tracer &trc) {
  for (impl::module_cache_map::iterator it = p->module_cache.begin();
      it != p->module_cache.end();
      ++it)
    trc("module-cache-item", it->second);
}

value importer::load(string const &name, bool binary_only) {
  impl::key_type key(name.to_string(), binary_only);

  impl::module_cache_map::iterator it = p->module_cache.find(key);
  if (it != p->module_cache.end())
    return it->second;

  object constructor = get_constructor<importer>();
  value preload = constructor.get_property("preload");

  if (preload.is_object()) {
    value loader = preload.get_object().get_property(name);
    if (loader.is_object()) {
      value result;
      if (!loader.is_null()) {
        local_root_scope scope;
        result = loader.get_object().apply(get_property("context").to_object());
      }
      return result;
    }
  }

  std::string so_name, js_name;
  so_name = process_name(name);
  js_name = process_name(name, true);
  
  //TODO: I'd like a version that throws an exception instead of assert traps
  value paths_v = get_property("paths").to_object();
  if (!paths_v.is_object())
    throw exception("Unable to get search paths or its not an object");

  array paths = paths_v.get_object();

  // TODO: We could probably do with an array class.
  size_t len = paths.get_length();
  for (size_t i=0; i < len; i++) {
    std::string path = paths.get_element(i).to_string().to_string();
    std::string fullpath = path + js_name;

    if (!binary_only) {
      std::ifstream file(fullpath.c_str(), std::ios::in | std::ios::binary);

      if (file.good()) {
        // Slurp in the file
        std::stringstream cbuf;
        cbuf << file.rdbuf();
        if (!file)
          // TODO: Is the information (fullpath) leak bad?
          throw exception( std::string("Error reading from ") + fullpath );

        // Execute the file
        std::string const &contents = cbuf.str();
        return get_current_context().evaluateInScope(contents.data(), 
            contents.size(), fullpath.c_str(), 1, 
            get_property("context").to_object());
      }
    }

    fullpath = path + so_name;

    if (boost::filesystem::exists(fullpath)) {
      // Load the .so
      void *module = dlopen(fullpath.c_str(), RTLD_LAZY);
      if (!module) {
        std::stringstream ss;
        ss << "Unable to load library '" << fullpath.c_str()
           << "': " << dlerror();
        throw exception(ss.str());
      }

      void *symbol = dlsym(module, "flusspferd_load");

      if (!symbol) {
        std::stringstream ss;
        ss << "Unable to load library '" << fullpath.c_str() 
           << "': " << dlerror();
        throw exception(ss.str());
      }

      flusspferd_load_t func = *(flusspferd_load_t*) &symbol;

      return func(get_property("context").to_object());
    }
  } 

  // We probably want to throw an exception here.
  std::stringstream ss;
  ss << "Unable to find library '";
  ss << name.c_str();
  ss << "' in [";
  ss << paths_v.to_string().c_str() << "]";
  throw exception(ss.str());
}

// Take 'foo.bar' as a flusspferd::string, check no path sep in it, and
// return '/foo/bar.js' or '/foo/libbar.so', etc. as a std::string
std::string importer::process_name(string const &name, bool for_script) {
  std::string p = name.to_string();

  if (p.find(DIRSEP1, 0) != std::string::npos &&
      p.find(DIRSEP2, 0) != std::string::npos) {
    throw exception("Path seperator not allowed in module name");
  }

  unsigned int pos = 0;
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

