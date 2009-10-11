// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
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

#include "flusspferd/version.hpp"
#include "flusspferd/load_core.hpp"
#include "flusspferd/create.hpp"

using namespace flusspferd;

void flusspferd::load_flusspferd_module(object container) {
  object exports = container.get_property_object("exports");

  exports.define_property(
    "version",
    value(flusspferd::version()),
    read_only_property | permanent_property);

#ifdef FLUSSPFERD_RELOCATABLE
  exports.define_property(
    "relocatable",
    value(true),
    read_only_property | permanent_property);

#else
  exports.define_property(
    "relocatable",
    value(false),
    read_only_property | permanent_property);

  exports.define_property(
    "installPrefix",
    value(INSTALL_PREFIX),
    read_only_property | permanent_property);

#endif

}

std::string flusspferd::version() {
  return FLUSSPFERD_VERSION;
}

