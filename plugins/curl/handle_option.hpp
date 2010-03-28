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
#ifndef FLUSSPFERD_PLUGINS_CURL_HANDLE_OPTION_HPP
#define FLUSSPFERD_PLUGINS_CURL_HANDLE_OPTION_HPP

#include "flusspferd/tracer.hpp"
#include <boost/any.hpp>
#include <curl/curl.h>

/*
 * cURL.Easy#options implementation:
 *
 * handle_option is the base class for option mappers to map between CURLOPT_*
 * and JavaScript. Data is stored as boost::any in EasyOpt::data. Getters/setters
 * are created (handle_option::getter/setter) on the fly in
 * EasyOpt::property_resolve.
 *
 * get_options() returns a map of all property names to their corresponding
 * handle_option.
 */

namespace curl {
  struct handle_option {
    virtual ~handle_option() =0;
    virtual flusspferd::object getter() const =0;
    virtual flusspferd::object setter() const =0;
    // initial data
    virtual boost::any data() const =0;
    virtual CURLoption what() const =0;
    // called during tracing to prevent the GC from collecting data
    virtual void trace(boost::any const &, flusspferd::tracer &) const;
    // called during destruction/cleanup of EasyOpt
    virtual void cleanup(boost::any &) const;
  };
}

#endif
