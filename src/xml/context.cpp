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

#include "flusspferd/xml/context.hpp"
#include "flusspferd/xml/document.hpp"
#include "flusspferd/string.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

namespace {

class namespaces : public native_object_base {
public:
  namespaces(object const &o, xmlHashTablePtr table);
  ~namespaces();

  void property_op(property_mode, value const &, value &);

  struct class_info : flusspferd::class_info {
    static char const *full_name() { return "XML.Context.Namespaces"; }
    typedef boost::mpl::bool_<false> constructible;
  };

private:
  xmlHashTablePtr table;
};

}

xml::context::context(object const &obj, call_context &x)
  : native_object_base(obj)
{
  document &doc = flusspferd::get_native<document>(x.arg[0].to_object());
  define_property("document", doc, read_only_property | permanent_property);

  xpath_ctx = xmlXPathNewContext(doc.c_obj());

  if (!xpath_ctx)
    throw exception("Could not initialise XPath context");

  xpath_ctx->nsHash = xmlHashCreate(1);

  if (!xpath_ctx->nsHash)
    throw exception("Could not initialise XPath context");

  object ns = create_native_object<namespaces>(object(), xpath_ctx->nsHash);
  ns.set_property("xml", string("http://www.w3.org/XML/1998/namespace"));
  define_property("ns", ns, read_only_property | permanent_property);
}

xml::context::~context() {
  xmlXPathFreeContext(xpath_ctx);
}

object xml::context::class_info::create_prototype() {
  object proto = create_object();

  return proto;
}

namespaces::namespaces(object const &obj, xmlHashTablePtr table)
  : native_object_base(obj), table(table)
{
}

namespaces::~namespaces() {}

void namespaces::property_op(
  property_mode mode, value const &id, value &data)
{
  local_root_scope scope;
  string name_ = id.to_string();
  xmlChar const *name = (xmlChar const*) name_.c_str();

  xmlChar *txt = 0;
  if (mode != property_get && mode != property_delete) {
    data = data.to_string();
    txt = xmlStrdup((xmlChar const *) data.get_string().c_str());
  }

  switch (mode) {
  case property_add:
    if (xmlHashAddEntry(table, name, txt) != 0)
      throw exception("Could not add entry to namespace hash table");
    break;
  case property_delete:
    if (xmlHashRemoveEntry(table, name, xmlHashDeallocator(xmlFree)) != 0)
      throw exception("Could not remove entry from namespace hash table");
    break;
  case property_set:
    if (xmlHashUpdateEntry(table, name, txt, xmlHashDeallocator(xmlFree))!=0)
      throw exception("Could not update entry in namespace hash table");
    break;
  case property_get:
    break;
  }
}
