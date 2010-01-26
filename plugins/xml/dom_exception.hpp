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

#ifndef FLUSSPFERD_XML_DOM_EXECEPTION__HPP
#define FLUSSPFERD_XML_DOM_EXECEPTION__HPP

#include <flusspferd.hpp>
#include "types.hpp"

#include <boost/fusion/include/make_vector.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace xml_plugin {

// Arabica's DOMException codes are out by one
#define enum_prop(x) (#x, constant, int(Arabica::DOM::DOMException:: x)+1)
FLUSSPFERD_CLASS_DESCRIPTION(
  dom_exception,
  (constructible, false)
  (full_name, "xml.DOMException")
  (constructor_name, "DOMException")
  (augment_prototype, 1)
  (constructor_properties,
    enum_prop(INDEX_SIZE_ERR)
    enum_prop(DOMSTRING_SIZE_ERR)
    enum_prop(HIERARCHY_REQUEST_ERR)
    enum_prop(WRONG_DOCUMENT_ERR)
    enum_prop(INVALID_CHARACTER_ERR)
    enum_prop(NO_DATA_ALLOWED_ERR)
    enum_prop(NO_MODIFICATION_ALLOWED_ERR)
    enum_prop(NOT_FOUND_ERR)
    enum_prop(NOT_SUPPORTED_ERR)
    enum_prop(INUSE_ATTRIBUTE_ERR)
    enum_prop(INVALID_STATE_ERR)
    enum_prop(SYNTAX_ERR)
    enum_prop(INVALID_MODIFICATION_ERR)
    enum_prop(NAMESPACE_ERR)
    enum_prop(INVALID_ACCESS_ERR)
  )
)
#undef enum_prop
{
public:
  typedef arabica_dom_exception wrapped_type;
  dom_exception(flusspferd::object const &proto, wrapped_type e);

  static void augment_prototype(flusspferd::object &proto);
};


#define XML_CB_TRY try
#define XML_CB_CATCH \
  catch (arabica_dom_exception &e) { \
    /* Convert the C++ dom_exception to a JS DOMException object */ \
    flusspferd::value v = flusspferd::create<dom_exception>( \
      boost::fusion::make_vector(e) \
    ); \
    throw flusspferd::exception(v); \
  }

} // namespace xml_plugin

#endif
