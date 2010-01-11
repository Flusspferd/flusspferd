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
#ifndef FLUSSPFERD_PLUGIN_CURL_LIST_OPTION_HPP
#define FLUSSPFERD_PLUGIN_CURL_LIST_OPTION_HPP

#include "handle_option.hpp"
#include "EasyOpt.hpp"
#include "Easy.hpp"
#include "flusspferd/array.hpp"
#include "flusspferd/create/function.hpp"
#include "flusspferd/create/array.hpp"

namespace curl {
  template<CURLoption What>
  struct list_option : handle_option {
    flusspferd::function getter() const {
      return flusspferd::create<flusspferd::method>("$get_", &get);
    }
    flusspferd::function setter() const {
      return flusspferd::create<flusspferd::method>("$set_", &set);
    }
    boost::any data() const {
      return data_t(flusspferd::create<flusspferd::array>(), 0x0);
    }
    CURLoption what() const { return What; }
    void trace(boost::any const &data, flusspferd::tracer &trc) const {
      data_t const &d = boost::any_cast<data_t const&>(data);
      if(d.second) {
        trc("list", d.first);
      }
    }
    void cleanup(boost::any &data) const {
      data_t &d = boost::any_cast<data_t&>(data);
      if(d.second) {
        curl_slist_free_all(d.second);
        d.second = 0x0;
      }
    }
  private:
    typedef std::pair<flusspferd::array,curl_slist*> data_t;
    static void reset(data_t &d) {
      if(d.second) {
        curl_slist_free_all(d.second);
        d.second = 0x0;
        d.first = flusspferd::create<flusspferd::array>();
      }
    }
    static flusspferd::array get(EasyOpt *o) {
      assert(o);
      return boost::any_cast<data_t&>(o->data[What]).first;
    }
    static void set(EasyOpt *o, flusspferd::array ain) {
      assert(o);
      data_t d(
          flusspferd::create<flusspferd::array>(
              boost::make_iterator_range(ain.begin(), ain.end())),
          0x0);
      try {
        for (flusspferd::array::iterator i = d.first.begin();
             i != d.first.end();
             ++i) {
          if(!i->is_string()) {
            throw exception("array data not a string");
          }
          curl_slist *r = curl_slist_append(
              d.second, i->get_string().c_str());
          if(!r) {
            throw exception("curl_slist_append");
          }
          d.second = r;
        }
      }
      catch(...) {
        reset(d);
        throw;
      }
      o->parent.do_setopt(What, d.second);
      data_t &old = boost::any_cast<data_t&>(o->data[What]);
      reset(old);
      old = d;
    }
  };
}
#endif
