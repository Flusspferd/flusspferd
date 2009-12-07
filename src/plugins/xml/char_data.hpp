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

#ifndef FLUSSPFERD_XML_CHAR_DATA_HPP
#define FLUSSPFERD_XML_CHAR_DATA_HPP

#include "node.hpp"


namespace xml_plugin {

FLUSSPFERD_CLASS_DESCRIPTION(
    character_data,
    (base, node)
    (constructible, false)
    (full_name, "xml.CharacterData")
    (constructor_name, "CharacterData")
    (properties,
      ("data", getter_setter, (getData, setData))
      ("length", getter, getLength)
    )
    (methods,
      ("substringData", bind, substringData)
      ("appendData", bind, appendData)
      ("insertData", bind, insertData)
      ("deleteData", bind, deleteData)
      ("replaceData", bind, replaceData)
    )
)
{

public:
  typedef arabica_char_data wrapped_type;

  character_data(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map);
  virtual ~character_data();

  // Property getters/setters
  string_type getData();
  void setData(string_type s);
  int getLength() { return impl_.getLength(); }

  // Methods
  string_type substringData(int offset, int count);
  void appendData(string_type arg);
  void insertData(int offset, string_type arg);
  void deleteData(int offset, int count);
  void replaceData(int offset, int count, string_type arg);

protected:
  wrapped_type impl_;
};



FLUSSPFERD_CLASS_DESCRIPTION(
    text,
    (base, character_data)
    (constructible, false)
    (full_name, "xml.Text")
    (constructor_name, "Text")
    (methods,
      ("splitText", bind, splitText)
    )
)
{
public:
  typedef arabica_text wrapped_type;
  text(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map);
  // Methods
  object splitText(int offset);
};


FLUSSPFERD_CLASS_DESCRIPTION(
    comment,
    (base, character_data)
    (constructible, false)
    (full_name, "xml.Comment")
    (constructor_name, "Comment")
)
{
public:
  typedef arabica_comment wrapped_type;
  comment(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map);
};

FLUSSPFERD_CLASS_DESCRIPTION(
    cdata,
    (base, text)
    (constructible, false)
    (full_name, "xml.CDATASection")
    (constructor_name, "CDATASection")
)
{
public:
  typedef arabica_cdata wrapped_type;
  cdata(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map);
};


} // namespace xml_plugin

#endif


