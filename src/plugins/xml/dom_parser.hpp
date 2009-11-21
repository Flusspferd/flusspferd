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
#include <SAX/helpers/CatchErrorHandler.hpp>
#include <DOM/io/Stream.hpp>

namespace xml_plugin {

FLUSSPFERD_CLASS_DESCRIPTION(
    dom_parser,
    (full_name, "xml.DOMParser")
    (constructor_name, "DOMParser")
    (constructor_methods,
      ("parse", bind_static, static_parse)
    )
    (methods,
      ("parse", bind, parse)
    )
) {
public:
  dom_parser(flusspferd::object const &proto, flusspferd::call_context &);
  dom_parser(flusspferd::object const &proto);
  virtual ~dom_parser();

  static flusspferd::object static_parse(flusspferd::value source);
  flusspferd::object parse(flusspferd::value source);
protected:
  typedef Arabica::SAX::InputSource<std::string> sax_source;

  Arabica::SAX2DOM::Parser<std::string> parser_;

  flusspferd::object parse_source(sax_source &stream);
};

}

#endif

