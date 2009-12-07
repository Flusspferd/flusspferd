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

#include <flusspferd.hpp>
#include <flusspferd/aliases.hpp>
#include "document.hpp"
#include "doctype.hpp"
#include "dom_implementation.hpp"
#include "dom_exception.hpp"

using namespace flusspferd;
using namespace flusspferd::aliases;
using namespace xml_plugin;


// Global/class statics make Ash a sad panda. Can't see a way around this one tho
/*static*/ weak_node_map dom_implementation::weak_node_map_;


dom_implementation::dom_implementation(object const &proto, wrapped_type const &impl)
  : base_type(proto),
    impl_(impl),
    master_node_map_(node_map::make(*this))
{
#ifdef DEBUG
  // We are setup to only have one of these! check it is so
  assert(weak_node_map_.expired());
#endif
  weak_node_map_ = master_node_map_;
}

bool dom_implementation::hasFeature(string_type feat, string_type ver) {
  return impl_.hasFeature(feat, ver);
}

object dom_implementation::createDocumentType(string_type qname, string_type pub_id, string_type sys_id) {
  XML_CB_TRY {
    return master_node_map_->get_node(
      impl_.createDocumentType(qname, pub_id, sys_id)
    );
  } XML_CB_CATCH
}

object dom_implementation::createDocument(string_type ns_uri, string_type qname,
                                          boost::optional<doctype&> doctype) {
  arabica_doctype dt = doctype
                     ? static_cast<arabica_doctype>(doctype->underlying_impl())
                     : arabica_doctype();

  XML_CB_TRY {
    return master_node_map_->get_node( impl_.createDocument( ns_uri, qname, dt ) );
  } XML_CB_CATCH
}
