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


#include <flusspferd/io/stream.hpp>
#include <flusspferd/binary.hpp>

#include <boost/format.hpp>
#include <fstream>
#include <sstream>

#include "parser.hpp"
#include "document.hpp"
#include "dom_implementation.hpp"

#include <Taggle/Taggle.hpp>


using boost::format;

using namespace flusspferd;
using namespace xml_plugin;

namespace xml_plugin {
  void load_parsers(object &exports) {
    load_class<xml_parser>(exports);
    load_class<html_parser>(exports);
  }
}

base_parser::base_parser(flusspferd::object const &proto)
  : base_type(proto)
{ }


object base_parser::parse(value source) {
  if (source.is_object()) {
    object o = source.get_object();

    if (is_native<io::stream>(o)) {
      io::stream &s = flusspferd::get_native<io::stream>(o);

      // TODO: Work out if the stream is readable or not!
      std::ifstream stream;
      dynamic_cast<std::ios&>(stream).rdbuf( s.streambuf() );
      sax_source is;
      is.setByteStream(stream);
      return parse_source(is);
    }
    /*else if (is_native<binary>(o)) {
      // Couldn't get this working. Compile errors
      binary &b = flusspferd::get_native<flusspferd::binary>(o);

      call_context c;
      c.arg.push_back(b);
      create<io::binary_stream>(c);
      root_object s(b_s);

      std::ifstream stream;
      dynamic_cast<std::ios&>(stream).rdbuf( b_s.streambuf() );
      sax_source is;
      is.setByteStream(stream);
      return parse_source(is);
    }*/
  }

  std::string str = source.to_std_string();

  security &sec = security::get();
  if (!sec.check_path(str, security::READ)) {
    throw exception(
      format("xml.Parser#parse: could not open file: 'denied by security' (%s)")
             % str
    );
  }

  sax_source is;
  is.setSystemId(str);

  return parse_source(is);
}

object base_parser::parse_string(std::string &str) {

  std::istringstream sb(str);
  sax_source is;
  is.setByteStream(sb);
  return parse_source(is);
}

#include <DOM/io/Stream.hpp>

object base_parser::parse_source(sax_source &is) {

  arabica_document const& doc = _parse(is);
  
  node_map_ptr map = dom_implementation::get_node_map().lock();
  if (!map)
    throw exception("Internal error: node_map has gone away");
  return map->get_node(doc);
}


xml_parser::xml_parser(flusspferd::object const &proto, flusspferd::call_context &)
  : base_type(proto)
{ }

xml_parser::xml_parser(flusspferd::object const &proto)
  : base_type(proto)
{ }

object xml_parser::static_parse(value source) {
  // Helper method. Create a parser and call the `parse` method
  return create<xml_parser>().parse(source);
}

object xml_parser::static_parse_string(std::string &source) {
  // Helper method. Create a parser and call the `parse` method
  return create<xml_parser>().parse_string(source);
}


arabica_document xml_parser::_parse(sax_source &is) {
  Arabica::SAX2DOM::Parser<string_type> parser;
  Arabica::SAX::CatchErrorHandler<string_type> eh;
  parser.setErrorHandler(eh);

  parser.parse(is);

  if (eh.errorsReported()) {
      throw exception( eh.errors() );
  }

  return parser.getDocument();
}


html_parser::html_parser(flusspferd::object const &proto, flusspferd::call_context &)
  : base_type(proto)
{ }

html_parser::html_parser(flusspferd::object const &proto)
  : base_type(proto)
{ }

object html_parser::static_parse(value source) {
  // Helper method. Create a parser and call the `parse` method
  return create<html_parser>().parse(source);
}

object html_parser::static_parse_string(std::string &source) {
  // Helper method. Create a parser and call the `parse` method
  return create<html_parser>().parse_string(source);
}


arabica_document html_parser::_parse(sax_source &is) {
  Arabica::SAX2DOM::Parser<string_type, Arabica::SAX::Taggle<string_type> > parser;
  Arabica::SAX::CatchErrorHandler<string_type> eh;
  parser.setErrorHandler(eh);

  parser.parse(is);

  if (eh.errorsReported()) {
      throw exception( eh.errors() );
  }

  return parser.getDocument();
}

