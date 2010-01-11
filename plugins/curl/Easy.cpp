// -*- mode:c++;coding:utf-8; -*- vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:enc=utf-8:
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
#include "defines.hpp"
#include "Easy.hpp"
#include "EasyOpt.hpp"
#include "exception.hpp"
#include "flusspferd/binary.hpp"
#include "flusspferd/tracer.hpp"

using curl::Easy;

size_t Easy::writefunction(void *ptr, size_t size, size_t nmemb, void *stream) {
  assert(stream);
  Easy &self = *reinterpret_cast<Easy*>(stream);
  if (self.writefunction_callback.is_null()) {
    return 0;
  } else {
    flusspferd::byte_array &data = flusspferd::create<flusspferd::byte_array>(
        boost::fusion::make_vector(
            reinterpret_cast<flusspferd::byte_array::element_type*>(ptr),
            size*nmemb));
    flusspferd::root_object d(data);
    flusspferd::arguments arg;
    arg.push_back(flusspferd::value(data));
    arg.push_back(flusspferd::value(size));
    flusspferd::root_value v(self.writefunction_callback.call(arg));
    return v.to_number();
  }
}

size_t Easy::readfunction(void *ptr, size_t size, size_t nmemb, void *stream) {
  assert(stream);
  Easy &self = *reinterpret_cast<Easy*>(stream);
  if (self.readfunction_callback.is_null()) {
    return CURL_READFUNC_ABORT;
  } else {
    flusspferd::byte_array &data = flusspferd::create<flusspferd::byte_array>(
        boost::fusion::vector2<flusspferd::binary::element_type*, std::size_t>(
            0x0, 0));
    flusspferd::root_object d(data);
    flusspferd::arguments arg;
    arg.push_back(flusspferd::value(data));
    arg.push_back(flusspferd::value(size));
    arg.push_back(flusspferd::value(nmemb));
    flusspferd::root_value v(self.readfunction_callback.call(arg));
    if(data.get_length() > size*nmemb) {
      throw exception("Out of Range");
    }
    std::copy(data.get_data().begin(), data.get_data().end(),
              static_cast<flusspferd::binary::element_type*>(ptr));
    return v.to_number();
  }
}

int Easy::seekfunction(void *stream, curl_off_t offset, int origin) {
  assert(stream);
  Easy &self = *reinterpret_cast<Easy*>(stream);
  if (self.readfunction_callback.is_null()) {
    return CURL_SEEKFUNC_CANTSEEK;
  } else {
    flusspferd::arguments arg;
    arg.push_back(flusspferd::value(offset));
    arg.push_back(flusspferd::value(origin));
    flusspferd::root_value v(self.seekfunction_callback.call(arg));
    return v.to_number();
  }
}

size_t Easy::headerfunction(void *ptr, size_t size, size_t nmemb, void *stream) {
  assert(stream);
  Easy &self = *reinterpret_cast<Easy*>(stream);
  if (self.headerfunction_callback.is_null()) {
    return 0;
  } else {
    flusspferd::byte_array &data = flusspferd::create<flusspferd::byte_array>(
        boost::fusion::make_vector(
            reinterpret_cast<flusspferd::byte_array::element_type*>(ptr),
            size*nmemb));
    flusspferd::root_object d(data);
    flusspferd::arguments arg;
    arg.push_back(flusspferd::value(data));
    arg.push_back(flusspferd::value(size));
    flusspferd::root_value v(self.headerfunction_callback.call(arg));
    return v.to_number();
  }
}

int Easy::progressfunction(
    void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
  assert(clientp);
  Easy &self = *reinterpret_cast<Easy*>(clientp);
  if (self.progressfunction_callback.is_null()) {
    return 0;
  } else {
    flusspferd::arguments arg;
    arg.push_back(flusspferd::value(dltotal));
    arg.push_back(flusspferd::value(dlnow));
    arg.push_back(flusspferd::value(ultotal));
    arg.push_back(flusspferd::value(ulnow));
    flusspferd::root_value v(self.progressfunction_callback.call(arg));
    return v.to_number();
  }
}

int Easy::debugfunction(CURL *hnd, curl_infotype i, char *buf, size_t len, void *p) {
  assert(p);
  Easy &self = *reinterpret_cast<Easy*>(p);
  if (self.progressfunction_callback.is_null() || hnd != self.handle) {
    return 0;
  } else {
    flusspferd::arguments arg;
    flusspferd::byte_array &data = flusspferd::create<flusspferd::byte_array>(
        boost::fusion::vector2<flusspferd::binary::element_type*, std::size_t>(
            reinterpret_cast<flusspferd::binary::element_type*>(buf),
            len));
    flusspferd::root_object d(data);
    arg.push_back(flusspferd::value(static_cast<int>(i)));
    arg.push_back(flusspferd::value(data));
    flusspferd::value v = self.debugfunction_callback.call(arg);
    return v.to_number();
  }
}

void Easy::trace(flusspferd::tracer &trc) {
  trc("options", opt);
  trc("writeFunction", writefunction_callback);
  trc("readFunction", readfunction_callback);
  trc("seekFunction", seekfunction_callback);
  trc("progressFunction", progressfunction_callback);
  trc("headerFunction", headerfunction_callback);
  trc("debugFunction", debugfunction_callback);
}

CURL *Easy::data() {
  return handle;
}

bool Easy::valid() {
  return handle;
}

CURL *Easy::get() {
  if (!handle)
    throw exception("CURL handle not valid!");
  return handle;
}

curl::EasyOpt &Easy::get_opt() {
  return opt;
}

Easy::Easy(flusspferd::object const &self, flusspferd::call_context&)
  : base_type(self), handle(curl_easy_init()), opt(EasyOpt::create(*this))
{
  if(!handle)
    throw exception("curl_easy_init");
}

Easy::Easy(flusspferd::object const &self, CURL *hnd)
  : base_type(self), handle(hnd), opt(EasyOpt::create(*this))
{
  assert(handle);
}

void Easy::cleanup() {
  if(handle) {
    curl_easy_cleanup(handle);
    handle = 0x0;
  }
}

Easy::~Easy() {
  cleanup();
}

void Easy::perform() {
  CURLcode res = curl_easy_perform(get());
  if (res != 0)
    throw exception(std::string("curl_easy_perform: ") +
                                curl_easy_strerror(res));
}

void Easy::reset() {
  curl_easy_reset(get());
  opt.clear();
}

std::string Easy::unescape(char const *input) {
  int len;
  char *const uesc = curl_easy_unescape(get(), input, 0, &len);
  if(!uesc) {
    throw exception("curl_easy_unescape");
  }
  std::string ret(uesc, len);
  curl_free(uesc);
  return ret;
}

std::string Easy::escape(char const *input) {
  char *const esc = curl_easy_escape(get(), input, 0);
  if(!esc) {
    throw exception("curl_easy_escape");
  }
  std::string ret(esc);
  curl_free(esc);
  return ret;
}

Easy &Easy::create(CURL *hnd) {
  return flusspferd::create<Easy>(boost::fusion::make_vector(hnd));
}

Easy &curl::wrap(CURL *hnd) {
  return Easy::create(hnd);
}

CURL *curl::unwrap(Easy &c) {
  return c.data();
}
