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

#include "flusspferd.hpp"
#include <curl/curl.h>

using namespace flusspferd;

// Put everything in an anon-namespace so typeid wont clash ever.
namespace {

FLUSSPFERD_CLASS_DESCRIPTION(
  curl,
  (full_name, "cURL")
  (constructor_name, "cURL")
  (augment_constructor, 1)
  (methods,
    ("setMethod", bind, set_method)
    ("perform", bind, perform)
  )
)
{
public:
  curl(object const &obj, call_context &x);
  virtual ~curl();

  static void augment_constructor(object &);

protected:
  CURL *curlHandle;
  char* error_buffer;

  typedef size_t (*curl_callback_t)(void *, size_t, size_t, void *);

  template<size_t (curl::*Method)(void*, size_t)>
  static size_t c_handle_curl(void *ptr, size_t size, size_t nmemb, void *stream);

  // Data is avilable
  size_t handle_curl_data( void* data, size_t size );
  // Header is available
  size_t handle_curl_header( void* data, size_t size );
  // Data is needed by curl (i.e. upload/request body)
  size_t handle_curl_data_needed( void* buff, size_t size );

  // TODO: Support setting CURLOPT_INFILESIZE for PUT methods

public: // JS methods
  void set_method(string &meth);
  int perform();
};

template<size_t (curl::*Method)(void*, size_t)>
size_t curl::c_handle_curl(void *ptr, size_t size, size_t nmemb, void *stream) {
  curl *self = static_cast<curl*>(stream);
  return (self->*Method)(ptr, size * nmemb);
}

FLUSSPFERD_LOADER(container) {
  load_class<curl>(container);
}

///////////////////////////
void curl::augment_constructor(object &ctor)
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

  ctor.define_property("versionHex", value((int)data->version_num),
                       read_only_property);
  ctor.define_property("versionStr", string( (const char*)data->version),
                       read_only_property);
}

///////////////////////////
curl::curl(object const &obj, call_context &) 
  : base_type(obj),
    error_buffer(NULL)
{
  curlHandle = curl_easy_init();

  if (!curlHandle)
    throw exception("curl_easy_init() failed");

  error_buffer = new char[CURL_ERROR_SIZE];
  
  curl_callback_t fn_ptr =  &curl::c_handle_curl<&curl::handle_curl_data>;
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, fn_ptr);

  fn_ptr =  &curl::c_handle_curl<&curl::handle_curl_header>;
  curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, fn_ptr);

  fn_ptr =  &curl::c_handle_curl<&curl::handle_curl_data_needed>;
  curl_easy_setopt(curlHandle, CURLOPT_READFUNCTION, fn_ptr);

  curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, this);
  curl_easy_setopt(curlHandle, CURLOPT_HEADERDATA, this);
  curl_easy_setopt(curlHandle, CURLOPT_READDATA, this);
  curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, error_buffer);
  curl_easy_setopt(curlHandle, CURLOPT_ENCODING, "");
}

///////////////////////////
curl::~curl() {
  delete [] error_buffer;
}

///////////////////////////
void curl::set_method(string &f_meth) {
  int ok = CURLE_OK;
  std::string meth = f_meth.to_string();

  if (meth == "GET")
    ok = curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1);
  else if (meth == "POST")
    ok = curl_easy_setopt(curlHandle, CURLOPT_POST, 1);
  else if (meth == "PUT")
    ok = curl_easy_setopt(curlHandle, CURLOPT_UPLOAD, 1);
  else if (meth == "HEAD")
    ok = curl_easy_setopt(curlHandle, CURLOPT_NOBODY, 1);
  else
    throw exception(("Unkown request method " + meth).c_str());

  if (ok != CURLE_OK)
    throw exception(error_buffer);
}

///////////////////////////
int curl::perform() {

  local_root_scope scope;
  // Create a copy of URL so that it lives as long as the perform call does
  // curl doesn't copy the CURLOPT_URL param, so it disapearing form under
  // it probably isn't very good
  string url = get_property("url").to_string();
  url = url.substr(0, url.length());

  // TODO: Make POST and PUT do READ_WRITE
  if (!security::get().check_url(url.to_string(), security::READ))
    throw exception("forbidden");

  if (curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str()) != CURLE_OK)
    throw exception(error_buffer);

  if (curl_easy_perform(curlHandle) != CURLE_OK)
    throw exception(error_buffer);

  long code = 0;
  curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &code);
  // Return value is status code
  return code;
}


size_t curl::handle_curl_data( void *data, size_t nbytes) {
  // See if we have a 'dataReceived' callback defined 
  value cb = get_property("dataReceived");

  if (cb.is_function()) {
    object obj = cb.to_object();

    apply(obj,
      create_native_object<blob>(
        object(), (unsigned char const *) data, nbytes));
  }

  return nbytes;
}

size_t curl::handle_curl_header( void *hdr, size_t nbytes ) {
  value cb = get_property("headerReceived");

  if (cb.is_function()) {
    object obj = cb.to_object();

    apply(obj, string((char const *) hdr, nbytes));
  }
  return nbytes;
}

size_t curl::handle_curl_data_needed( void*, size_t ) {
  return 0;
}

} // End of anon namespace
