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


#include <flusspferd.hpp>
#include <flusspferd/io/stream.hpp>

#include <boost/format.hpp>
#include <fstream>

#include "dom_parser.hpp"
#include "document.hpp"
#include "dom_implementation.hpp"


using boost::format;

using namespace flusspferd;
using namespace xml_plugin;


dom_parser::dom_parser(flusspferd::object const &proto, flusspferd::call_context &)
  : base_type(proto)
{ }

dom_parser::dom_parser(flusspferd::object const &proto)
  : base_type(proto)
{ }

dom_parser::~dom_parser() {
}

object dom_parser::static_parse(value source) {
  // Helper method. Create a parser and call the `parse` method
  return create<dom_parser>().parse(source);
}


object dom_parser::parse(value source) {
  if (source.is_object()) {
    object o = source.get_object();

    if (is_native<flusspferd::io::stream>(o)) {
      flusspferd::io::stream &s = flusspferd::get_native<flusspferd::io::stream>(o);

      // TODO: Work out if the stream is readable or not!
      std::ifstream stream;
      dynamic_cast<std::ios&>(stream).rdbuf( s.streambuf() );
      sax_source is;
      is.setByteStream(stream);
      return parse_source(is);
    }
  }

  std::string str = source.to_std_string();

  security &sec = security::get();
  if (!sec.check_path(str, security::READ)) {
    throw exception( boost::str(
      format("xml.DOMParser#parse: could not open file: 'denied by security' (%s)")
             % str
    ) );
  }

  sax_source is;
  is.setSystemId(str);

  return parse_source(is);
}


object dom_parser::parse_source(sax_source &is) {
  Arabica::SAX::CatchErrorHandler<std::string> eh;
  parser_.setErrorHandler(eh);

  parser_.parse(is);

  if (eh.errorsReported()) {
      throw exception( eh.errors() );
  }

  document::wrapped_type const &doc = parser_.getDocument();
  node_map_ptr map = dom_implementation::get_node_map().lock();
  if (!map)
    throw exception("Internal error: node_map has gone away");
  return map->get_node(doc);
}
