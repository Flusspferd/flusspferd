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
#ifndef FLUSSPFERD_PLUGIN_CURL_HTTP_POST_OPTION_HPP
#define FLUSSPFERD_PLUGIN_CURL_HTTP_POST_OPTION_HPP

#include "handle_option.hpp"
#include "EasyOpt.hpp"
#include "Easy.hpp"
#include "flusspferd/array.hpp"
#include "flusspferd/create/function.hpp"
#include "flusspferd/create/array.hpp"

namespace curl {
  struct http_post_option : handle_option {
    static CURLoption const What;
    flusspferd::function getter() const {
      return flusspferd::create<flusspferd::method>("$get_httppost", &get);
    }
    flusspferd::function setter() const {
      return flusspferd::create<flusspferd::method>("$set_httppost", &set);
    }
    boost::any data() const {
      return data_t(flusspferd::create<flusspferd::array>(), 0x0);
    }
    CURLoption what() const { return What; }
    void trace(boost::any const &data, flusspferd::tracer &trc) const {
      data_t const &d = boost::any_cast<data_t const&>(data);
      if(d.second) {
        trc("httppost", d.first);
      }
    }
    void cleanup(boost::any &data) const {
      reset(boost::any_cast<data_t&>(data));        
    }
  private:
    typedef std::pair<flusspferd::array, curl_httppost*> data_t;
    static void reset(data_t &d) {
      if(d.second) {
        curl_formfree(d.second);
        d.second = 0x0;
      }
    }

    static flusspferd::object get(EasyOpt *o) {
      assert(o);
      return boost::any_cast<data_t&>(o->data[What]).first;
    }

    static char const *get_data_ptr(flusspferd::value v) {
      if(!v.is_string()) { // TODO binary stuff
        throw exception("data is not a string", "TypeError");
      }
      return v.get_string().c_str();
    }

    static void add_data(
        std::vector<curl_forms> &forms,
        CURLformoption option,
        char const *name,
        flusspferd::object o) {
      if(!o.has_property(name))
        return;
      curl_forms form;
      form.option = option;
      form.value = get_data_ptr(o.get_property(name));
      forms.push_back(form);
    }

    static void add_length(
        std::vector<curl_forms> &forms,
        CURLformoption option,
        char const *name,
        flusspferd::object o) {
      if(!o.has_property(name))
        return;
      curl_forms form;
      form.option = option;
      flusspferd::value v = o.get_property(name);
      if(!v.is_int()) {
        throw exception("data is not an int", "TypeError");
      }
      // yay curl's api is weird
      form.value = reinterpret_cast<char const*>(v.get_int());
      forms.push_back(form);
    }

    static void object2form(
        flusspferd::object o, curl_httppost *&post, curl_httppost *&last) {
      if(!o.has_property("name")) {
        throw exception("object has no `name' property", "TypeError");
      }
      std::vector<curl_forms> forms;
      curl_forms form;
      add_data(forms, CURLFORM_PTRCONTENTS, "contents", o);
      add_length(forms, CURLFORM_CONTENTSLENGTH, "contentslength", o);
      add_length(forms, CURLFORM_CONTENTSLENGTH, "contentsLength", o); // alt
      add_data(forms, CURLFORM_FILECONTENT, "filecontent", o);
      add_data(forms, CURLFORM_FILECONTENT, "fileContent", o); // alt
      add_data(forms, CURLFORM_FILE, "file", o);
      add_data(forms, CURLFORM_CONTENTTYPE, "contenttype", o);
      add_data(forms, CURLFORM_CONTENTTYPE, "contentType", o); // alt
      add_data(forms, CURLFORM_FILENAME, "filename", o);
      add_data(forms, CURLFORM_FILENAME, "fileName", o); // alt
      // TODO buffer ...
      form.option = CURLFORM_END;
      form.value = 0x0;
      forms.push_back(form);
      CURLFORMcode ret = curl_formadd(
          &post, &last,
          CURLFORM_PTRNAME, get_data_ptr(o.get_property("name")),
          CURLFORM_ARRAY, &forms[0], CURLFORM_END); // TODO free data
      if(ret != 0) {
        std::stringstream sstr;
        sstr << "curl_formadd failed! " << static_cast<unsigned>(ret);
        throw exception(sstr.str());
      }
    }
    static void set(EasyOpt *o, flusspferd::object val) {
      assert(o);
      curl_httppost *post = 0x0;
      curl_httppost *last = 0x0;
      if(val.is_array()) {
        // copy the array
        flusspferd::array const ain(val);
        flusspferd::root_array a(
            flusspferd::create<flusspferd::array>(
                boost::make_iterator_range(ain.begin(), ain.end())));
        for (flusspferd::array::iterator i = a.begin(); i != a.end(); ++i) {
          if(!i->is_object()) {
            throw exception("array member not an object", "TypeError");
          }
          object2form(i->get_object(), post, last);
        }
        data_t &old = boost::any_cast<data_t&>(o->data[What]);
        reset(old);
        old = data_t(a, post);
      }
      else {
        flusspferd::root_array a(flusspferd::create<flusspferd::array>());
        a.push(val);
        object2form(val, post, last);
        data_t &old = boost::any_cast<data_t&>(o->data[What]);
        reset(old);
        old = data_t(a, post);
      }
      o->parent.do_setopt(What, post);
    }
  };
}

#endif
