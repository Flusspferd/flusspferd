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
#include "EasyOpt.hpp"
#include "flusspferd/create/native_object.hpp"

curl::EasyOpt::EasyOpt(flusspferd::object const &self, Easy &parent)
  : base_type(self), parent(parent)
{ }

curl::EasyOpt::~EasyOpt() {
  for(options_map_t::const_iterator i = get_options().begin();
      i != get_options().end();
      ++i) {
    data_map_t::iterator j = data.find(i->second->what());
    if (j != data.end())
      i->second->cleanup(j->second);
  }
}

curl::EasyOpt &curl::EasyOpt::create(curl::Easy &p) {
  return flusspferd::create<EasyOpt>(boost::fusion::make_vector(boost::ref(p)));
}

void curl::EasyOpt::clear() {
  for(options_map_t::const_iterator i = get_options().begin();
      i != get_options().end();
      ++i)
    if (has_property(i->first))
      delete_property(i->first);
  data.clear();
}

void curl::EasyOpt::trace(flusspferd::tracer &trc) {
  for (data_map_t::const_iterator i = data.begin(); i != data.end(); ++i) {
    for (options_map_t::const_iterator j = get_options().begin();
         j != get_options().end();
         ++j) {
      if (j->second->what() == i->first) {
        j->second->trace(i->second, trc);
        break;
      }
    }
  }
}

bool curl::EasyOpt::property_resolve(flusspferd::value const &id, unsigned) {
  std::string const name = id.to_std_string();
  options_map_t::const_iterator const i = get_options().find(name);
  if(i != get_options().end()) {
    flusspferd::property_attributes attr(
        flusspferd::no_property_flag,
        i->second->getter(),
        i->second->setter());
    define_property(id.get_string(), attr);
    data[i->second->what()] = i->second->data();
    return true;
  }
  else {
    // TODO: throw exception if option is unkown?
    return false;
  }
}
