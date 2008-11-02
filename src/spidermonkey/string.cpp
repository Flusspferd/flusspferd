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

#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/value.hpp"
#include "flusspferd/spidermonkey/context.hpp"
#include <js/jsapi.h>
#include <cstring>

using namespace flusspferd;

Impl::string_impl::string_impl(char const *s)
 : str(JS_NewStringCopyZ(Impl::current_context(), s))
{
  if (!str)
    throw exception("Could not create string");
}

Impl::string_impl::string_impl(char const *s, std::size_t n)
  : str(JS_NewStringCopyN(Impl::current_context(), s, n))
{
  if (!str)
    throw exception("Could not create string");
}

Impl::string_impl::string_impl(char16_t const *s, std::size_t n)
  : str(JS_NewUCStringCopyN(Impl::current_context(), s, n))
{
  if (!str)
    throw exception("Could not create string");
}

Impl::string_impl::string_impl(value const &v)
  : str(JS_ValueToString(Impl::current_context(),
                         Impl::get_jsval(const_cast<value&>(v))))
{
  if (!str)
    throw exception("Could not create string");
}

namespace {
  JSString *get_string(string const &s) {
    return Impl::get_string(const_cast<string&>(s));
  }
}

string::string() { }
string::string(value const &v) : Impl::string_impl(v) { }
string::string(char const *s) : Impl::string_impl(s) { }
string::string(std::string const &s)
  : Impl::string_impl(s.data(), s.size()) { }
string::string(std::basic_string<char16_t> const &s)
  : Impl::string_impl(s.data(), s.size()) { }
string::~string() { }

string &string::operator=(string const &o) {
  string_impl::operator=(o);
  return *this;
}

std::size_t string::length() const {
  assert(get_string(*this));
  return JS_GetStringLength(get_string(*this));
}

char const *string::c_str() const {
  assert(get_string(*this));
  return JS_GetStringBytes(get_string(*this));
}

std::string string::to_string() const {
  assert(get_string(*this));
  return JS_GetStringBytes(get_string(*this));
}

std::basic_string<char16_t> string::to_utf16_string() const {
  JSString *str = get_string(*this);
  assert(str);
  std::size_t len = JS_GetStringLength(str);
  jschar *text = JS_GetStringChars(str);
  return std::basic_string<char16_t>(text, len);
}

bool operator==(string const &lhs, std::string const &rhs) {
  return JS_CompareStrings(get_string(lhs), get_string(rhs)) == 0;
}

bool operator<(string const &lhs, string const &rhs) {
  return JS_CompareStrings(get_string(lhs), get_string(rhs)) < 0;
}

string string::substr(size_t start, size_t length) {
  JSContext *ctx = Impl::current_context();
  JSString *new_string =
    JS_NewDependentString(ctx, get_string(*this), start, length);
  if (!new_string)
    throw exception("Could not create substring");
  return Impl::wrap_string(new_string);
}
