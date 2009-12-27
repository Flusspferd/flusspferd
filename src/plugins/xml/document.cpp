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

#include "document.hpp"
#include "dom_implementation.hpp"
#include "node_list.hpp"
#include "dom_exception.hpp"

using namespace flusspferd;
using namespace flusspferd::aliases;
using namespace xml_plugin;

namespace xml_plugin {
  void load_doc_classes(object &exports) {
    load_class<document>(exports);
    load_class<document_fragment>(exports);
  }
}

document::document(object const &proto, call_context &)
  : base_type(proto)
{
}


document::document(object const &proto, wrapped_type const &doc, weak_node_map map)
  : base_type(proto, doc, map),
    doc_(doc)
{
}

document::~document() {
}

object document::getDoctype() {
  return get_node(doc_.getDoctype());
}

object document::getImplementation() {
  node_map_ptr map = node_map_.lock();
  if (!map)
    throw exception("Internal error: node_map has gone away");

  return map->get_dom_implementation();
}

object document::getDocumentElement() {
  return get_node(doc_.getDocumentElement());
}

object document::createElement(string_type tag_name) {
  XML_CB_TRY {
    return get_node(doc_.createElement(tag_name));
  } XML_CB_CATCH
}

object document::createDocumentFragment() {
  XML_CB_TRY {
    return get_node(doc_.createDocumentFragment());
  } XML_CB_CATCH
}

object document::createTextNode(string_type data) {
  XML_CB_TRY {
    return get_node(doc_.createTextNode(data));
  } XML_CB_CATCH
}

object document::createComment(string_type data) {
  XML_CB_TRY {
    return get_node(doc_.createComment(data));
  } XML_CB_CATCH
}

object document::createCDATASection(string_type data) {
  XML_CB_TRY {
    return get_node(doc_.createCDATASection(data));
  } XML_CB_CATCH
}

object document::createProcessingInstruction(string_type target, string_type data) {
  XML_CB_TRY {
    return get_node(doc_.createProcessingInstruction(target, data));
  } XML_CB_CATCH
}

object document::createAttribute(string_type name) {
  XML_CB_TRY {
    return get_node(doc_.createAttribute(name));
  } XML_CB_CATCH
}

object document::createEntityReference(string_type name) {
  XML_CB_TRY {
    return get_node(doc_.createEntityReference(name));
  } XML_CB_CATCH
}

object document::getElementsByTagName(std::string tagname) {
  XML_CB_TRY {
    return create<node_list>( make_vector(
      doc_.getElementsByTagName(tagname),
      node_map_
    ) );
  } XML_CB_CATCH
}

object document::importNode(node &arg, bool deep) {
  XML_CB_TRY {
    return get_node(doc_.importNode(arg.underlying_impl(), deep));
  } XML_CB_CATCH
}

object document::createElementNS(string_type ns_uri, string_type local_name) {
  XML_CB_TRY {
    return get_node(doc_.createElementNS(ns_uri, local_name));
  } XML_CB_CATCH
}

object document::getElementsByTagNameNS(string_type ns_uri, string_type local_name) {
  XML_CB_TRY {
    return create<node_list>( make_vector(
      doc_.getElementsByTagNameNS(ns_uri, local_name),
      node_map_
    ) );
  } XML_CB_CATCH
}

object document::getElementById(string_type id) {
  XML_CB_TRY {
    return get_node(doc_.getElementById(id));
  } XML_CB_CATCH
}




document_fragment::document_fragment(object const &proto, wrapped_type const &node, weak_node_map map)
  : base_type(proto, node, map)
{ }
