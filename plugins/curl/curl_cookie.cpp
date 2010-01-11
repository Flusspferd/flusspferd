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
#include "exception.hpp"
#include "curl_cookie.hpp"
#include <curl/curl.h>
#include <boost/thread.hpp>

namespace { boost::mutex cookie; }

curl::cURL_cookie::cURL_cookie(object const &obj)
  : base_type(obj)
{
  boost::mutex::scoped_lock lock(cookie);
  CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
  if(ret != 0)
    throw curl::exception("curl_global_init") << curlcode_info(ret);
}

curl::cURL_cookie::~cURL_cookie() {
  boost::mutex::scoped_lock lock(cookie);
  curl_global_cleanup();
}

