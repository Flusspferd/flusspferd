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
#include "flusspferd/xml/html_document.hpp"
#include "flusspferd/xml/html_push_parser.hpp"
#include "flusspferd/function_adapter.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/security.hpp"
#include <boost/thread/once.hpp>
#include <boost/preprocessor.hpp>
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

static void xml_constants(object);
static void html_constants(object);

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
  create_native_function(XML, "parse", &parse_file);

  object XPath = load_class<xpath_context>(XML);

  object HTML = flusspferd::create_object();

  load_class<html_document>(HTML);
  load_class<html_push_parser>(HTML);
  create_native_function(HTML, "parseBlob", &html_parse_blob);
  create_native_function(HTML, "parse", &html_parse_file);

  html_constants(HTML);

  XML.define_property(
    "HTML",
    HTML,
    object::read_only_property | object::permanent_property);

  xml_constants(XML);

  container.define_property(
    "XML",
    XML,
    object::read_only_property | object::permanent_property);

  return XML;
}

static void xml_constants(object XML) {
#define CONST(x) \
  XML.define_property( \
    BOOST_PP_STRINGIZE(x), \
    BOOST_PP_CAT(XML_, x), \
    object::read_only_property | object::permanent_property) \
  /**/

  // parser constants
  CONST(PARSE_RECOVER);
  CONST(PARSE_NOENT);
  CONST(PARSE_DTDLOAD);
  //CONST(PARSE_DTDADDR);
  CONST(PARSE_DTDVALID);
  CONST(PARSE_NOERROR);
  CONST(PARSE_NOWARNING);
  CONST(PARSE_PEDANTIC);
  CONST(PARSE_NOBLANKS);
  CONST(PARSE_SAX1);
  CONST(PARSE_XINCLUDE);
  CONST(PARSE_NONET);
  CONST(PARSE_NODICT);
  CONST(PARSE_NSCLEAN);
  CONST(PARSE_NOCDATA);
  CONST(PARSE_NOXINCNODE);
  CONST(PARSE_COMPACT);
  //CONST(PARSE_OLD10);
  //CONST(PARSE_NOBASEFIX);
  //CONST(PARSE_HUGE);

#undef CONST
}

static void html_constants(object HTML) {
#define CONST(x) \
  HTML.define_property( \
    BOOST_PP_STRINGIZE(x), \
    BOOST_PP_CAT(HTML_, x), \
    object::read_only_property | object::permanent_property) \
  /**/

  // parser constants
  CONST(PARSE_RECOVER);
  CONST(PARSE_NOERROR);
  CONST(PARSE_NOWARNING);
  CONST(PARSE_PEDANTIC);
  CONST(PARSE_NOBLANKS);
  CONST(PARSE_NONET);
  CONST(PARSE_COMPACT);

#undef CONST
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
