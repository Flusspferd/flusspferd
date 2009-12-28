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

#ifndef FLUSSPFERD_XML_DOM_PARSER_HPP
#define FLUSSPFERD_XML_DOM_PARSER_HPP

#include <DOM/SAX2DOM/SAX2DOM.hpp>

#include "types.hpp"

namespace xml_plugin {

FLUSSPFERD_CLASS_DESCRIPTION(
    base_parser,
    (constructible, false)
    (full_name, "xml.Parser")
    (constructor_name, "Parser")
    (methods,
      ("parse", bind, parse)
      ("parseString", bind, parse_string)
    )
) {
public:
  base_parser(flusspferd::object const &proto);
  virtual ~base_parser() {};

  flusspferd::object parse(flusspferd::value source);
  flusspferd::object parse_string(std::string &str);
protected:
  typedef Arabica::SAX::InputSource<string_type> sax_source;

  flusspferd::object parse_source(sax_source &stream);
  virtual arabica_document _parse(sax_source &stream) = 0;
};

FLUSSPFERD_CLASS_DESCRIPTION(
    html_parser,
    (base, base_parser)
    (full_name, "xml.HTMLParser")
    (constructor_name, "HTMLParser")
    (constructor_methods,
      ("parse", bind_static, static_parse)
      ("parseString", bind_static, static_parse_string)
    )
) {
public:
  html_parser(flusspferd::object const &proto, flusspferd::call_context &);
  html_parser(flusspferd::object const &proto);

  static flusspferd::object static_parse(flusspferd::value source);
  static flusspferd::object static_parse_string(std::string &str);
protected:
  arabica_document _parse(sax_source &stream);
};

FLUSSPFERD_CLASS_DESCRIPTION(
    xml_parser,
    (base, base_parser)
    (full_name, "xml.XMLParser")
    (constructor_name, "XMLParser")
    (constructor_methods,
      ("parse", bind_static, static_parse)
      ("parseString", bind_static, static_parse_string)
    )
) {
public:
  xml_parser(flusspferd::object const &proto, flusspferd::call_context &);
  xml_parser(flusspferd::object const &proto);

  static flusspferd::object static_parse(flusspferd::value source);
  static flusspferd::object static_parse_string(std::string &str);
protected:
  arabica_document _parse(sax_source &stream);
};

}

#endif


