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

#ifndef FLUSSPFERD_XML_TYPES_HPP
#define FLUSSPFERD_XML_TYPES_HPP

#include <DOM/Node.hpp>
#include <DOM/DOMImplementation.hpp>
#include <DOM/Document.hpp>
#include <DOM/DocumentFragment.hpp>
#include <DOM/DocumentType.hpp>
#include <DOM/Element.hpp>
#include <DOM/CharacterData.hpp>
#include <DOM/Text.hpp>
#include <DOM/Comment.hpp>
#include <DOM/CDATASection.hpp>
#include <DOM/Attr.hpp>
#include <DOM/Notation.hpp>
#include <DOM/Entity.hpp>
#include <DOM/EntityReference.hpp>
#include <DOM/ProcessingInstruction.hpp>

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace xml_plugin {

typedef std::string string_type;
typedef Arabica::DOM::Node<string_type> arabica_node;
typedef Arabica::DOM::DOMImplementation<string_type> arabica_dom_impl;
typedef Arabica::DOM::Document<string_type> arabica_document;
typedef Arabica::DOM::DocumentFragment<string_type> arabica_doc_fragment;
typedef Arabica::DOM::DocumentType<string_type> arabica_doctype;
typedef Arabica::DOM::Element<string_type> arabica_element;
typedef Arabica::DOM::CharacterData<string_type> arabica_char_data;
typedef Arabica::DOM::Text<string_type> arabica_text;
typedef Arabica::DOM::Comment<string_type> arabica_comment;
typedef Arabica::DOM::CDATASection<string_type> arabica_cdata;
typedef Arabica::DOM::Attr<string_type> arabica_attr;
typedef Arabica::DOM::Notation<string_type> arabica_notation;
typedef Arabica::DOM::Entity<string_type> arabica_entity;
typedef Arabica::DOM::EntityReference<string_type> arabica_entity_ref;
typedef Arabica::DOM::ProcessingInstruction<string_type> arabica_pi;

class node_map;

typedef boost::weak_ptr<node_map> weak_node_map;
typedef boost::shared_ptr<node_map> node_map_ptr;
class node;
class node_list;
class named_node_map;
class document;
class element;

}

#endif;

