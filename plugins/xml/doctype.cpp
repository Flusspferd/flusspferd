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

#include <flusspferd/aliases.hpp>

#include "named_node_map.hpp"
#include "doctype.hpp"
#include "dom_exception.hpp"

using namespace flusspferd;
using namespace flusspferd::aliases;
using namespace xml_plugin;

namespace xml_plugin {
  void load_doctype_class(object &exports) {
    load_class<doctype>(exports);
  }
}

doctype::doctype(object const &proto, wrapped_type const &impl, weak_node_map map)
  : base_type(proto, impl, map),
    impl_(impl)
{ }

string_type doctype::getName() {
  XML_CB_TRY {
    return impl_.getName();
  } XML_CB_CATCH
}

object doctype::getEntities() {
  XML_CB_TRY {
    return create<named_node_map>( make_vector( impl_.getEntities(), node_map_) );
  } XML_CB_CATCH
}

object doctype::getNotations() {
  XML_CB_TRY {
    return create<named_node_map>( make_vector( impl_.getNotations(), node_map_) );
  } XML_CB_CATCH
}

string_type doctype::getPublicId() {
  XML_CB_TRY {
    return impl_.getPublicId();
  } XML_CB_CATCH
}

string_type doctype::getSystemId() {
  XML_CB_TRY {
    return impl_.getSystemId();
  } XML_CB_CATCH
}

string_type doctype::getInternalSubset() {
  XML_CB_TRY {
    return impl_.getInternalSubset();
  } XML_CB_CATCH
}
