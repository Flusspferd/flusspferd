// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Ash Berlin

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

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/native_object_base.hpp"
#include "flusspferd/string.hpp"

#include <curl/curl.h>

using namespace flusspferd;

// Put everything in an anon-namespace so typeid wont clash ever.
namespace {

class curl : public native_object_base {
public:
  struct class_info : public flusspferd::class_info {
    static char const *full_name() { return "cURL"; }
    typedef boost::mpl::bool_<false> constructible;
    static char const *constructor_name() { return "cURL"; }
    static void augment_constructor(object &ctor);
  };
};

///////////////////////////
// import hook
extern "C" value flusspferd_load(object container)
{
  return load_class<curl>(container);
}

///////////////////////////
void curl::class_info::augment_constructor(object &ctor)
{
  curl_version_info_data* data = curl_version_info(CURLVERSION_NOW);

  if (!data) 
    // Very unlikely to happen, but deal with it
    throw exception("Unable to get curl_version_info");

  array protocols = create_array();
  ctor.define_property("supportedProtocols", protocols, read_only_property );

  size_t i = 0;
  for(const char* const* p = data->protocols; *p != NULL; p++, i++) {
    protocols.set_element(i, string( *p ) );
  }
  // Make the properties of the array read only too
  //protocols.seal(false);
  // Arse. Can't in 1.8

  ctor.define_property("versionHex", (int)data->version_num, read_only_property);
  ctor.define_property("versionStr", string( (const char*)data->version), read_only_property);

}

} // End of anon namespace
