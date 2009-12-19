// -*- mode:c++;coding:utf-8; -*- vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:enc=utf-8:
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

#include "exception.hpp"
#include "EasyOpt.hpp"
#include "Easy.hpp"
#include "defines.hpp"
#include "curl_cookie.hpp"
#include "flusspferd/array.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/binary.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/modules.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/class_description.hpp"
#include "flusspferd/property_iterator.hpp"

#include "flusspferd/create/array.hpp"

#include <sstream>
#include <curl/curl.h>

#include <boost/range/iterator_range.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/any.hpp>

using namespace flusspferd;

namespace curl {
  namespace bf = boost::fusion;
}

FLUSSPFERD_LOADER_SIMPLE(cURL) {
  local_root_scope scope;

  load_class<curl::cURL_cookie>(cURL);

  create<curl::cURL_cookie>(
      param::_name = "$cURL_cookie",
      param::_container = cURL);

  load_class<curl::EasyOpt>(cURL);
  load_class<curl::Easy>(cURL);

  cURL.define_properties(read_only_property | permanent_property)
    ("version", string(curl_version()))
    ("PROTO_HTTP", value(static_cast<int>(CURLPROTO_HTTP)))
    ("PROTO_HTTPS", value(static_cast<int>(CURLPROTO_HTTPS)))
    ("PROTO_FTP", value(static_cast<int>(CURLPROTO_FTP)))
    ("PROTO_FTPS", value(static_cast<int>(CURLPROTO_FTPS)))
    ("PROTO_SCP", value(static_cast<int>(CURLPROTO_SCP)))
    ("PROTO_SFTP", value(static_cast<int>(CURLPROTO_SFTP)))
    ("PROTO_TELNET", value(static_cast<int>(CURLPROTO_TELNET)))
    ("PROTO_LDAP", value(static_cast<int>(CURLPROTO_LDAP)))
    ("PROTO_LDAPS", value(static_cast<int>(CURLPROTO_LDAPS)))
    ("PROTO_DICT", value(static_cast<int>(CURLPROTO_DICT)))
    ("PROTO_FILE", value(static_cast<int>(CURLPROTO_FILE)))
    ("PROTO_TFTP", value(static_cast<int>(CURLPROTO_TFTP)))
    ("PROTO_ALL", value(static_cast<int>(CURLPROTO_ALL)))
    ("INFO_TEXT", value(static_cast<int>(CURLINFO_TEXT)))
    ("INFO_HEADER_IN", value(static_cast<int>(CURLINFO_HEADER_IN)))
    ("INFO_HEADER_OUT", value(static_cast<int>(CURLINFO_HEADER_OUT)))
    ("INFO_DATA_IN", value(static_cast<int>(CURLINFO_DATA_IN)))
    ("INFO_DATA_OUT", value(static_cast<int>(CURLINFO_DATA_OUT)))
    ("PROXY_HTTP", value(static_cast<int>(CURLPROXY_HTTP)))
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 4)
    ("PROXY_HTTP_1_0", value(static_cast<int>(CURLPROXY_HTTP_1_0)))
#endif
    ("PROXY_SOCKS4", value(static_cast<int>(CURLPROXY_SOCKS4)))
    ("PROXY_SOCKS5", value(static_cast<int>(CURLPROXY_SOCKS5)))
    ("PROXY_SOCKS4A", value(static_cast<int>(CURLPROXY_SOCKS4A)))
    ("PROXY_SOCKS5_HOSTNAME", value(static_cast<int>(CURLPROXY_SOCKS5_HOSTNAME)))
    ("NETRC_OPTIONAL", value(static_cast<int>(CURL_NETRC_OPTIONAL)))
    ("NETRC_IGNORED", value(static_cast<int>(CURL_NETRC_IGNORED)))
    ("NETRC_REQUIRED", value(static_cast<int>(CURL_NETRC_REQUIRED)))
    ("AUTH_BASIC", value(static_cast<int>(CURLAUTH_BASIC)))
    ("AUTH_DIGEST", value(static_cast<int>(CURLAUTH_DIGEST)))
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 3)
    ("AUTH_DIGEST_IE", value(static_cast<int>(CURLAUTH_DIGEST_IE)))
#endif
    ("AUTH_GSSNEGOTIATE", value(static_cast<int>(CURLAUTH_GSSNEGOTIATE)))
    ("AUTH_NTLM", value(static_cast<int>(CURLAUTH_NTLM)))
    ("AUTH_ANY", value(static_cast<int>(CURLAUTH_ANY)))
    ("AUTH_ANYSAFE", value(static_cast<int>(CURLAUTH_ANYSAFE)))
    ("REDIR_POST_301", value(static_cast<int>(CURL_REDIR_POST_301)))
    ("REDIR_POST_302", value(static_cast<int>(CURL_REDIR_POST_302)))
    ("REDIR_POST_ALL", value(static_cast<int>(CURL_REDIR_POST_ALL)))
    ("HTTP_VERSION_NONE", value(static_cast<int>(CURL_HTTP_VERSION_NONE)))
    ("HTTP_VERSION_1_0", value(static_cast<int>(CURL_HTTP_VERSION_1_0)))
    ("HTTP_VERSION_1_1", value(static_cast<int>(CURL_HTTP_VERSION_1_1)))
    ("USESSL_NONE", value(static_cast<int>(CURLUSESSL_NONE)))
    ("USESSL_TRY", value(static_cast<int>(CURLUSESSL_TRY)))
    ("USESSL_CONTROL", value(static_cast<int>(CURLUSESSL_CONTROL)))
    ("USESSL_ALL", value(static_cast<int>(CURLUSESSL_ALL)))
    ("FTPAUTH_DEFAULT", value(static_cast<int>(CURLFTPAUTH_DEFAULT)))
    ("FTPAUTH_SSL", value(static_cast<int>(CURLFTPAUTH_SSL)))
    ("FTPAUTH_TLS", value(static_cast<int>(CURLFTPAUTH_TLS)))
    ("FTPSSL_CCC_NONE", value(static_cast<int>(CURLFTPSSL_CCC_NONE)))
    ("FTPSSL_CCC_PASSIVE", value(static_cast<int>(CURLFTPSSL_CCC_PASSIVE)))
    ("FTPSSL_CCC_ACTIVE", value(static_cast<int>(CURLFTPSSL_CCC_ACTIVE)))
    ("FTPMETHOD_MULTICWD", value(static_cast<int>(CURLFTPMETHOD_MULTICWD)))
    ("FTPMETHOD_NOCWD", value(static_cast<int>(CURLFTPMETHOD_NOCWD)))
    ("FTPMETHOD_SINGLECWD", value(static_cast<int>(CURLFTPMETHOD_SINGLECWD)))
    ("IPRESOLVE_WHATEVER", value(static_cast<int>(CURL_IPRESOLVE_WHATEVER)))
    ("IPRESOLVE_V4", value(static_cast<int>(CURL_IPRESOLVE_V4)))
    ("IPRESOLVE_V6", value(static_cast<int>(CURL_IPRESOLVE_V6)))
    ("SSLVERSION_DEFAULT", value(static_cast<int>(CURL_SSLVERSION_DEFAULT)))
    ("SSLVERSION_TLSv1", value(static_cast<int>(CURL_SSLVERSION_TLSv1)))
    ("SSLVERSION_SSLv2", value(static_cast<int>(CURL_SSLVERSION_SSLv2)))
    ("SSLVERSION_SSLv3", value(static_cast<int>(CURL_SSLVERSION_SSLv3)))
    ("SEEKFUNC_OK", value(CURL_SEEKFUNC_OK))
    ("SEEKFUNC_FAIL", value(CURL_SEEKFUNC_FAIL))
    ("SEEKFUNC_CANTSEEK", value(CURL_SEEKFUNC_CANTSEEK));
}
