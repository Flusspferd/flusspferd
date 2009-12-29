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
#ifndef FLUSSPFERD_PLUGINS_CURL_EASYOPT_HPP
#define FLUSSPFERD_PLUGINS_CURL_EASYOPT_HPP

#include "get_options.hpp"
#include "flusspferd/class_description.hpp"
#include <boost/unordered_map.hpp>
#include <boost/any.hpp>
#include <curl/curl.h>

namespace curl {
  class Easy;

  // the class behind cURL.Easy#options.
  FLUSSPFERD_CLASS_DESCRIPTION
  (
      EasyOpt,
      (constructor_name, "EasyOpt")
      (full_name, "cURL.Easy.EasyOpt")
      (constructible, false)
  )
  {
  public: // TODO this should be private
    typedef boost::unordered_map<CURLoption, boost::any> data_map_t;
    data_map_t data;
    Easy &parent;

  public:
    EasyOpt(flusspferd::object const &self, Easy &parent);

    ~EasyOpt();

    static EasyOpt &create(Easy &p);

    void clear();

  protected:
    bool property_resolve(flusspferd::value const &id, unsigned access);

    void trace(flusspferd::tracer &trc);
  };
}

#endif
