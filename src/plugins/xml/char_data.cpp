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
#include <flusspferd/aliases.hpp>

#include "char_data.hpp"
#include "dom_exception.hpp"

using namespace flusspferd;
using namespace flusspferd::aliases;
using namespace xml_plugin;

character_data::character_data(object const &proto, wrapped_type const &node, weak_node_map map)
  : base_type(proto, node, map),
    impl_(node)
{ }

character_data::~character_data()
{ }


string_type character_data::getData() {
  XML_CB_TRY {
    return impl_.getData();
  } XML_CB_CATCH
}

void character_data::setData(string_type s) {
  XML_CB_TRY {
    impl_.setData(s);
  } XML_CB_CATCH
}

string_type character_data::substringData(int offset, int count) {
  XML_CB_TRY {
    return impl_.substringData(offset, count);
  } XML_CB_CATCH
}

void character_data::appendData(string_type arg) {
  XML_CB_TRY {
    impl_.appendData(arg);
  } XML_CB_CATCH
}

void character_data::insertData(int offset, string_type arg) {
  XML_CB_TRY {
    impl_.insertData(offset, arg);
  } XML_CB_CATCH
}

void character_data::deleteData(int offset, int count) {
  XML_CB_TRY {
    impl_.deleteData(offset, count);
  } XML_CB_CATCH
}

void character_data::replaceData(int offset, int count, string_type arg) {
  XML_CB_TRY {
    impl_.replaceData(offset, count, arg);
  } XML_CB_CATCH
}




text::text(object const &proto, wrapped_type const &node, weak_node_map map)
  : base_type(proto, node, map)
{ }

object text::splitText(int offset) {
  XML_CB_TRY {
    return get_node(static_cast<arabica_text&>(impl_).splitText(offset));
  } XML_CB_CATCH
}



cdata::cdata(object const &proto, wrapped_type const &node, weak_node_map map)
  : base_type(proto, node, map)
{ }



comment::comment(object const &proto, wrapped_type const &node, weak_node_map map)
  : base_type(proto, node, map)
{ }

