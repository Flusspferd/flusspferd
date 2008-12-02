// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_XML_TEXT_HPP
#define FLUSSPFERD_XML_TEXT_HPP

#include "node.hpp"
#include <boost/noncopyable.hpp>
#include <boost/mpl/size_t.hpp>
#include <libxml/tree.h>

namespace flusspferd { namespace xml {

#define FLUSSPFERD_XML_TAG(id, text) \
  struct id { \
    static char const *name() { return text; } \
    static char const *full_name() { return "XML." text; } \
  }

FLUSSPFERD_XML_TAG(text_tag, "Text");
FLUSSPFERD_XML_TAG(comment_tag, "Comment");
FLUSSPFERD_XML_TAG(cdata_section_tag, "CDataSection");

#undef FLUSSPFERD_XML_TAG

template<typename Tag>
class general_text : public node {
public:
  struct class_info : node::class_info {
    static char const *full_name() { return Tag::full_name(); }

    static char const *constructor_name() { return Tag::name(); }
    typedef boost::mpl::size_t<2> constructor_arity;

    static object create_prototype();
  };

  general_text(object const &, call_context &);
  general_text(object const &, xmlNodePtr doc);
  ~general_text();

private: // JS methods

private: // JS properties
};

typedef general_text<text_tag> text;
typedef general_text<comment_tag> comment;
typedef general_text<cdata_section_tag> cdata_section;

}}

#endif
