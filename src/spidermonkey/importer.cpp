// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:

#include "flusspferd/spidermonkey/importer.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"

#include <iostream>
#include <fstream>
#include <string>

// Wrap the windows calls to the *nix equivalents
#ifndef _WIN32
# include <dlfcn.h>
# define DIRSEP1 "/"
# define DIRSEP2 "\0"
# define SHLIBPREFIX "lib"
# ifdef APPLE
#  define SHLIBSUFFIX ".dylib"
# else
#  define SHLIBSUFFIX ".so"
# endif
#else
# define DIRSEP1 "\\"
# define DIRSEP2 "/"
# define SHLIBPREFIX 0
# define SHLIBSUFFIX ".dll"
# include <windows.h>
# include <errno.h>
# define dlopen(x,y) (void*)LoadLibrary(x)
# define dlsym(x,y) (void*)GetProcAddress((HMODULE)x,y)
# define dlclose(x) FreeLibrary((HMODULE)x)
const char* dlerror() {
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

  create_native_method(proto, "load", 2);

  return proto;
}

importer::importer(call_context &x) {
}

importer::~importer() {
}

void importer::post_initialize() {

  register_native_method("load", &importer::load);

  // Store search paths
  set_property("paths", create_array());
}

value importer::load(string const &name, bool binary_only) {
  std::string so_name, js_name;
  so_name = process_name(name);
  js_name = process_name(name, true);
  
  //TODO: I'd like a version that throws an exception instead of assert traps
  value paths_v = get_property("paths").to_object();
  if (!paths_v.is_object())
    throw exception("Unable to get search paths or its not an object");

  object paths = paths_v.get_object();

  // TODO: We could probably do with an array class.
  int len = paths.get_property("length").to_number();
  for (int i=0; i < len; i++) {
    std::string path = paths.get_property(i).to_string().to_string();

    if (!binary_only) {
      std::ifstream file( (path + js_name).c_str(), std::ios::in | std::ios::binary );

      if (file.good()) {
        // Execute the file
        return string(js_name);
      }

    }

    // Load the .so
    std::ifstream file( (path + so_name).c_str(), std::ios::in | std::ios::binary );

    if (file.good()) {
      // Execute the file
      return string(so_name);
    }
  }

  // We probably want to throw an exception here.
  return value();
}

std::string importer::process_name(string const &name, bool for_script) {
  std::string p = name.to_string();

  if (p.find(DIRSEP1, 0) != std::string::npos &&
      p.find(DIRSEP2, 0) != std::string::npos) {
    throw exception("path seperator allowed in module name");
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
