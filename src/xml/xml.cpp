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

#include "flusspferd/xml/xml.hpp"
#include "flusspferd/xml/xpath_context.hpp"
#include "flusspferd/xml/parse.hpp"
#include "flusspferd/xml/push_parser.hpp"
#include "flusspferd/xml/node.hpp"
#include "flusspferd/xml/document.hpp"
#include "flusspferd/xml/text.hpp"
#include "flusspferd/xml/namespace.hpp"
#include "flusspferd/xml/reference.hpp"
#include "flusspferd/xml/attribute.hpp"
#include "flusspferd/xml/processing_instruction.hpp"
#include "flusspferd/function_adapter.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/security.hpp"
#include <boost/thread/once.hpp>
#include <libxml/xmlIO.h>
#include <libxml/xpath.h>

using namespace flusspferd;
using namespace flusspferd::xml;

extern "C" value flusspferd_load(object container)
{
  return load_xml(container);
}

static void safety_io_callbacks();

static void once() {
  LIBXML_TEST_VERSION
  xmlXPathInit();
  safety_io_callbacks();
}

static boost::once_flag once_flag = BOOST_ONCE_INIT;

object flusspferd::xml::load_xml(object container) {
  local_root_scope scope;

  value previous = container.get_property("XML");

  if (previous.is_object())
    return previous.to_object();

  boost::call_once(once_flag, &once);

  object XML = flusspferd::create_object();

  load_class<node>(XML);
  load_class<document>(XML);
  load_class<text>(XML);
  load_class<comment>(XML);
  load_class<cdata_section>(XML);
  load_class<reference_>(XML);
  load_class<processing_instruction>(XML);
  load_class<attribute_>(XML);
  load_class<namespace_>(XML);

  load_class<push_parser>(XML);

  create_native_function(XML, "parseBlob", &parse_blob);
  create_native_function(XML, "parseFile", &parse_file);

  object XPath = flusspferd::create_object();

  load_class<xpath_context>(XPath);

  XML.define_property(
    "XPath",
    XPath,
    object::read_only_property | object::permanent_property);

  container.define_property(
    "XML",
    XML,
    object::read_only_property | object::permanent_property);

  return XML;
}

template<int (*OldMatch)(char const *), unsigned mode>
static int safety_match(char const *name) {
  if (!OldMatch(name))
    return 0;

  std::string path(name);

  std::size_t nonletter = path.find_first_not_of(
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  if (nonletter != 0 && nonletter != std::string::npos &&
      path[nonletter] == ':')
    return flusspferd::security::get().check_url(path, mode);
  else
    return flusspferd::security::get().check_path(path, mode);
}

static void safety_io_callbacks() {
  xmlCleanupInputCallbacks();
  xmlCleanupOutputCallbacks();

#define REG_INPUT(name) \
  xmlRegisterInputCallbacks( \
    safety_match<name ## Match, security::READ>, \
    name ## Open, \
    name ## Read, \
    name ## Close) \
  /**/

  REG_INPUT(xmlFile);
  REG_INPUT(xmlIOHTTP);

  // disable any output
  xmlRegisterOutputCallbacks(0, 0, 0, 0);
}
