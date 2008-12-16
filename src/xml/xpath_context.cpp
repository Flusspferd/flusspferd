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

#include "flusspferd/xml/xpath_context.hpp"
#include "flusspferd/xml/document.hpp"
#include "flusspferd/xml/node.hpp"
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
    static char const *full_name() { return "XML.XPath.Context.Namespaces"; }
    typedef boost::mpl::bool_<false> constructible;
  };

private:
  xmlHashTablePtr table;
};

}

xpath_context::xpath_context(object const &obj, call_context &x)
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

  define_native_property(
    "current", permanent_shared_property, &xpath_context::prop_current);

  register_native_method("", &xpath_context::eval);
}

xpath_context::~xpath_context() {
  xmlXPathFreeContext(xpath_ctx);
}

object xpath_context::class_info::create_prototype() {
  object proto = create_object();

  return proto;
}

void xpath_context::eval(call_context &x) {
  xmlXPathObjectPtr obj;

  if (x.arg[0].is_string()) {
    string expr = x.arg[0];

    obj = xmlXPathEval((xmlChar const *) expr.c_str(), xpath_ctx);

    if (!obj)
      throw exception("Could not evaluate XPath expression");

    x.result = true;
  } else {
    throw exception("Does not work now.");
  }

  switch (obj->type) {
  case XPATH_UNDEFINED:
    x.result = value();
    break;
  case XPATH_BOOLEAN:
    x.result = bool(obj->boolval);
    break;
  case XPATH_NUMBER:
    x.result = obj->floatval;
    break;
  case XPATH_STRING:
    x.result = string((char const *) obj->stringval);
    break;
  case XPATH_NODESET:
    {
      xmlNodeSetPtr nodeset = obj->nodesetval;
      array arr = create_array(nodeset->nodeNr);
      for (int i = 0; i < nodeset->nodeNr; ++i)
        arr.set_element(i, node::create(nodeset->nodeTab[i]));
      x.result = arr;
    }
    break;
  default:
    throw exception("XPath object type not supported");
    break;
  }
}

void xpath_context::prop_current(property_mode mode, value &data) {
  switch (mode) {
  case property_set:
    if (data.is_void_or_null()) {
      xpath_ctx->node = 0;
    } else {
      xmlNodePtr ptr = node::c_from_js(data.to_object());
      if (!ptr)
        throw exception("Not a valid XML node");
      xpath_ctx->node = ptr;
    }
    // !! fall thru !!
  case property_get:
    data = node::create(xpath_ctx->node);
    break;
  default: break;
  }
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
