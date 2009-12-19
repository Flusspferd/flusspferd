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
#include "handle_option.hpp"
#include "get_options.hpp"
#include "integer_option.hpp"
#include "string_option.hpp"
#include "list_option.hpp"
#include "function_option.hpp"
#include "http_post_option.hpp"
#include "EasyOpt.hpp"
#include "Easy.hpp"
#include "defines.hpp"
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

#include <boost/assign/ptr_map_inserter.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/any.hpp>

using namespace flusspferd;

namespace curl {
  namespace bf = boost::fusion;

  /*
   * cURL.Easy#options implementation:
   *
   * handle_option is the base class for option mappers to map between CURLOPT_*
   * and JavaScript. Data is stored as boost::any in EasyOpt::data. Getters/setters
   * are created (handle_option::getter/setter) on the fly in EasyOpt::property_resolve.
   * get_options() returns a map of all property names to their corresponding handle_option.
   */

  options_map_t const &get_options() {
    /* elisp helper (+ keyboard macros):
       (defun insopt (type name)
       (interactive "sTyp: \nsOpt: \n")
       (if (string= (downcase type) "i")
       (setq type "integer")
       (if (string= (downcase type) "s")
       (setq type "string")
       (if (string= (downcase type) "l")
       (setq type "list"))))
       (setq name (replace-regexp-in-string "^CURLOPT_" "" name))
       (insert (concat "ptr_map_insert< " type "_option<CURLOPT_" name "> >(map)(\"" (downcase name) "\");")))
       (defun insreg (begin end type)
       (interactive "r\nsTyp: ")
       (insopt type (buffer-substring begin end))
       (kill-region begin end)
       (c-indent-line-or-region))
    */
    static options_map_t map;
    if(map.empty()) {
      using namespace boost::assign;
      // this is used to auto generate some documentation. See gen-doc.js!
      // BEGIN DOC{
      /* DOC{
         cURL.Easy#options -> EasyOpt

         The options property is used to set connection options (see [curl_easy_setopt](http://curl.haxx.se/libcurl/c/curl_easy_setopt.html)).

         ## Example #
         %      c.options.url = 'http://www.google.com'

         ## Options #
         The following options are supported. See the libcurl documentation links.
         Flag parameters are class constants of [[cURL]] without the leading `CURL(_)`.
         (e.g. `CURLPROTO_HTTP` -> `cURL.PROTO_HTTP`)

         + integer: boolean or number
         + string: string
         + list: array of strings
         }DOC
      */
      // BEHAVIOR OPTIONS
      ptr_map_insert< integer_option<CURLOPT_VERBOSE> >(map)("verbose");
      ptr_map_insert< integer_option<CURLOPT_HEADER> >(map)("header");
      ptr_map_insert< integer_option<CURLOPT_NOPROGRESS> >(map)("noprogress");
      ptr_map_insert< integer_option<CURLOPT_NOSIGNAL> >(map)("nosignal");
      // CALLBACK OPTIONS
      /* DOC{
         + writefunction, function(buffer,size). [CURLOPT_WRITEFUNCTION](http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPT_WRITEFUNCTION)
         Callback should except buffer and size as parameter. Buffer is a [[binary.ByteArray]] and size the size of each object. Should return the length of the buffer if everything is okay.
         }DOC*/
      ptr_map_insert< function_option<CURLOPT_WRITEFUNCTION,
        CURLOPT_WRITEDATA, &Easy::writefunction_callback> >(map)
        ("writefunction");
      /* DOC{
         + readfunction, function(buffer,size,nmemb). [CURLOPT_READFUNCTION](http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPT_READFUNCTION)
         }DOC*/
      ptr_map_insert< function_option<CURLOPT_READFUNCTION,
        CURLOPT_READDATA, &Easy::readfunction_callback> >(map)
        ("readfunction");
      /* DOC{
         + seekfunction, function(offset,origin). [CURLOPT_SEEKFUNCTION](http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPT_SEEKFUNCTION)
         }DOC*/
      ptr_map_insert< function_option<CURLOPT_SEEKFUNCTION,
        CURLOPT_SEEKDATA, &Easy::seekfunction_callback> >(map)
        ("seekfunction");
      /* DOC{
         + progressfunction, function(dltotal,dlnow,ultotal,ulnow). [CURLOPT_SEEKFUNCTION](http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPT_PROGRESSFUNCTION)
         }DOC*/
      ptr_map_insert< function_option<CURLOPT_PROGRESSFUNCTION,
        CURLOPT_PROGRESSDATA, &Easy::progressfunction_callback> >(map)
        ("progressfunction");
      /* DOC{
         + headerfunction, function(data,size). [CURLOPT_HEADERFUNCTION](http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPT_HEADERFUNCTION)
         }DOC*/
      ptr_map_insert< function_option<CURLOPT_HEADERFUNCTION,
        CURLOPT_HEADERDATA, &Easy::headerfunction_callback> >(map)
        ("headerfunction");
      /* DOC{
         + debugfunction, function(infotype,buffer). [CURLOPT_DEBUGFUNCTION](http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPT_DEBUGFUNCTION)
         }DOC*/
      ptr_map_insert< function_option<CURLOPT_DEBUGFUNCTION,
        CURLOPT_DEBUGDATA, &Easy::debugfunction_callback> >(map)
        ("debugfunction");
      // ERROR OPTIONS
      ptr_map_insert< integer_option<CURLOPT_FAILONERROR> >(map)("failonerror");
      // NETWORK OPTIONS
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 4)
      ptr_map_insert< integer_option<CURLOPT_PROTOCOLS> >(map)("protocols");
      ptr_map_insert< integer_option<CURLOPT_REDIR_PROTOCOLS> >(map)("redirProtocols");
#endif
      ptr_map_insert< string_option<CURLOPT_URL> >(map)("url");
      ptr_map_insert< string_option<CURLOPT_PROXY> >(map)("proxy");
      ptr_map_insert< integer_option<CURLOPT_PROXYPORT> >(map)("proxyport");
      ptr_map_insert< integer_option<CURLOPT_PROXYTYPE> >(map)("proxytype"); // See cURL.PROXY_
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 4)
      ptr_map_insert< string_option<CURLOPT_NOPROXY> >(map)("noproxy");
#endif
      ptr_map_insert< integer_option<CURLOPT_HTTPPROXYTUNNEL> >(map)("httpProxyTunnel");
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 4)
      ptr_map_insert< string_option<CURLOPT_SOCKS5_GSSAPI_SERVICE> >(map)("socks5GssapiService");
      ptr_map_insert< integer_option<CURLOPT_SOCKS5_GSSAPI_NEC> >(map)("socks5GssapiNec");
#endif
      ptr_map_insert< string_option<CURLOPT_INTERFACE> >(map)("interface");
      ptr_map_insert< integer_option<CURLOPT_LOCALPORT> >(map)("localport");
      ptr_map_insert< integer_option<CURLOPT_LOCALPORTRANGE> >(map)("localportrange");
      ptr_map_insert< integer_option<CURLOPT_DNS_CACHE_TIMEOUT> >(map)("dnsCacheTimeout");
      ptr_map_insert< integer_option<CURLOPT_DNS_USE_GLOBAL_CACHE> >(map)("dnsUseGlobalCache");
      ptr_map_insert< integer_option<CURLOPT_BUFFERSIZE> >(map)("buffersize");
      ptr_map_insert< integer_option<CURLOPT_PORT> >(map)("port");
      ptr_map_insert< integer_option<CURLOPT_TCP_NODELAY> >(map)("tcpNoDelay");
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 4)
      ptr_map_insert< integer_option<CURLOPT_ADDRESS_SCOPE> >(map)("addressScope");
#endif
      // NAMES and PASSWORDS OPTIONS (Authentication)
      ptr_map_insert< integer_option<CURLOPT_NETRC> >(map)("netrc");
      ptr_map_insert< string_option<CURLOPT_NETRC_FILE> >(map)("netrcFile");
      ptr_map_insert< string_option<CURLOPT_USERPWD> >(map)("userpwd");
      ptr_map_insert< string_option<CURLOPT_PROXYUSERPWD> >(map)("proxyuserpwd");
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 1)
      ptr_map_insert< string_option<CURLOPT_USERNAME> >(map)("username");
      ptr_map_insert< string_option<CURLOPT_PASSWORD> >(map)("password");
      ptr_map_insert< string_option<CURLOPT_PROXYUSERNAME> >(map)("proxyusername");
      ptr_map_insert< string_option<CURLOPT_PROXYPASSWORD> >(map)("proxypassword");
#endif
      ptr_map_insert< integer_option<CURLOPT_HTTPAUTH> >(map)("httpauth"); // See cURL.AUTH_*
      ptr_map_insert< integer_option<CURLOPT_PROXYAUTH> >(map)("proxyauth");
      // HTTP OPTIONS
      ptr_map_insert< integer_option<CURLOPT_AUTOREFERER> >(map)("autoreferer");
      ptr_map_insert< string_option<CURLOPT_ENCODING> >(map)("encoding");
      ptr_map_insert< integer_option<CURLOPT_FOLLOWLOCATION> >(map)("followlocation");
      ptr_map_insert< integer_option<CURLOPT_UNRESTRICTED_AUTH> >(map)("unrestrictedAuth");
      ptr_map_insert< integer_option<CURLOPT_MAXREDIRS> >(map)("maxredirs");
      ptr_map_insert< integer_option<CURLOPT_POSTREDIR> >(map)("postredir"); // See cURL.REDIR_*
      ptr_map_insert< integer_option<CURLOPT_POST> >(map)("post");
      ptr_map_insert< string_option<CURLOPT_POSTFIELDS> >(map)("postfields");
      ptr_map_insert< integer_option<CURLOPT_POSTFIELDSIZE> >(map)("postfieldsize");
      /* DOC{
         + httppost, array of objects|object [CURLOPT_HTTPPOST](http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPT_HTTPPOST)
         Expects a single object or an array of objects containing a property `name` and either `contents`, `contentslength`, `filecontent`, `file`, `contenttype` or `filename`. See libcurl docs.
         }DOC*/
      ptr_map_insert< http_post_option >(map)("httppost");
      ptr_map_insert< string_option<CURLOPT_REFERER> >(map)("referer");
      ptr_map_insert< string_option<CURLOPT_USERAGENT> >(map)("userAgent");
      ptr_map_insert< list_option<CURLOPT_HTTPHEADER> >(map)("httpheader");
      ptr_map_insert< list_option<CURLOPT_HTTP200ALIASES> >(map)("http200aliases");
      ptr_map_insert< string_option<CURLOPT_COOKIE> >(map)("cookie");
      ptr_map_insert< string_option<CURLOPT_COOKIEFILE> >(map)("cookieFile");
      ptr_map_insert< string_option<CURLOPT_COOKIEJAR> >(map)("cookiejar");
      ptr_map_insert< integer_option<CURLOPT_COOKIESESSION> >(map)("cookiesession");
      ptr_map_insert< string_option<CURLOPT_COOKIELIST> >(map)("cookielist");
      ptr_map_insert< integer_option<CURLOPT_HTTPGET> >(map)("httpGet");
      ptr_map_insert< integer_option<CURLOPT_HTTP_VERSION> >(map)("httpVersion"); // See cURL.HTTP_VERSION_*
      ptr_map_insert< integer_option<CURLOPT_IGNORE_CONTENT_LENGTH> >(map)("ignoreContentLength");
      ptr_map_insert< integer_option<CURLOPT_HTTP_CONTENT_DECODING> >(map)("httpContentDecoding");
      ptr_map_insert< integer_option<CURLOPT_HTTP_TRANSFER_DECODING> >(map)("httpTransferDecoding");
      // TFTP OPTIONS
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 4)
      ptr_map_insert< integer_option<CURLOPT_TFTP_BLKSIZE> >(map)("tftpBlksize");
#endif
      // FTP OPTIONS
      ptr_map_insert< string_option<CURLOPT_FTPPORT> >(map)("ftpPort");
      ptr_map_insert< list_option<CURLOPT_QUOTE> >(map)("quote");
      ptr_map_insert< list_option<CURLOPT_POSTQUOTE> >(map)("postquote");
      ptr_map_insert< list_option<CURLOPT_PREQUOTE> >(map)("prequote");
      ptr_map_insert< integer_option<CURLOPT_DIRLISTONLY> >(map)("dirListOnly");
      ptr_map_insert< integer_option<CURLOPT_APPEND> >(map)("append");
      ptr_map_insert< integer_option<CURLOPT_FTP_USE_EPRT> >(map)("ftpUseEprt");
      ptr_map_insert< integer_option<CURLOPT_FTP_USE_EPSV> >(map)("ftpUseEpsv");
      ptr_map_insert< integer_option<CURLOPT_FTP_CREATE_MISSING_DIRS> >(map)("ftpCreateMissingDirs");
      ptr_map_insert< integer_option<CURLOPT_FTP_RESPONSE_TIMEOUT> >(map)("ftpResponseTimeout");
      ptr_map_insert< string_option<CURLOPT_FTP_ALTERNATIVE_TO_USER> >(map)("ftpAltnerativeToUser");
      ptr_map_insert< integer_option<CURLOPT_FTP_SKIP_PASV_IP> >(map)("ftpSkipPasvIp");
      ptr_map_insert< integer_option<CURLOPT_USE_SSL> >(map)("usessl");
      ptr_map_insert< integer_option<CURLOPT_FTPSSLAUTH> >(map)("ftpsslauth");
      ptr_map_insert< integer_option<CURLOPT_FTP_SSL_CCC> >(map)("ftpSslCcc");
      ptr_map_insert< string_option<CURLOPT_FTP_ACCOUNT> >(map)("ftpAccount");
      ptr_map_insert< integer_option<CURLOPT_FTP_FILEMETHOD> >(map)("ftpFilemethod");
      // PROTOCOL OPTIONS
      ptr_map_insert< integer_option<CURLOPT_TRANSFERTEXT> >(map)("transfertext");
      ptr_map_insert< integer_option<CURLOPT_PROXY_TRANSFER_MODE> >(map)("proxyTransferMode");
      ptr_map_insert< integer_option<CURLOPT_CRLF> >(map)("crlf");
      ptr_map_insert< string_option<CURLOPT_RANGE> >(map)("range");
      ptr_map_insert< integer_option<CURLOPT_RESUME_FROM> >(map)("resumeFrom");
      ptr_map_insert< string_option<CURLOPT_CUSTOMREQUEST> >(map)("customrequest");
      ptr_map_insert< integer_option<CURLOPT_FILETIME> >(map)("filetime");
      ptr_map_insert< integer_option<CURLOPT_NOBODY> >(map)("nobody");
      ptr_map_insert< integer_option<CURLOPT_INFILESIZE> >(map)("infilesize");
      ptr_map_insert< integer_option<CURLOPT_UPLOAD> >(map)("upload");
      ptr_map_insert< integer_option<CURLOPT_MAXFILESIZE> >(map)("maxfilesize");
      ptr_map_insert< integer_option<CURLOPT_TIMECONDITION> >(map)("timecondition");
      ptr_map_insert< integer_option<CURLOPT_TIMEVALUE> >(map)("timevalue");
      // CONNECTION OPTIONS
      ptr_map_insert< integer_option<CURLOPT_TIMEOUT> >(map)("timeout");
      ptr_map_insert< integer_option<CURLOPT_TIMEOUT_MS> >(map)("timeoutMS");
      ptr_map_insert< integer_option<CURLOPT_LOW_SPEED_LIMIT> >(map)("lowSpeedLimit");
      ptr_map_insert< integer_option<CURLOPT_LOW_SPEED_TIME> >(map)("lowSpeedTime");
      ptr_map_insert< integer_option<CURLOPT_MAX_SEND_SPEED_LARGE> >(map)("maxSendSpeed");
      ptr_map_insert< integer_option<CURLOPT_MAX_RECV_SPEED_LARGE> >(map)("maxRecvSpeed");
      ptr_map_insert< integer_option<CURLOPT_MAXCONNECTS> >(map)("maxconnects");
      ptr_map_insert< integer_option<CURLOPT_FRESH_CONNECT> >(map)("freshConnect");
      ptr_map_insert< integer_option<CURLOPT_FORBID_REUSE> >(map)("forbidReuse");
      ptr_map_insert< integer_option<CURLOPT_CONNECTTIMEOUT> >(map)("connecttimeout");
      ptr_map_insert< integer_option<CURLOPT_CONNECTTIMEOUT_MS> >(map)("connecttimeoutMS");
      ptr_map_insert< integer_option<CURLOPT_IPRESOLVE> >(map)("ipresolve"); // See cURL.IPRESOLVE_*
      ptr_map_insert< integer_option<CURLOPT_CONNECT_ONLY> >(map)("connectOnly");
      // SSL and SECURITY OPTIONS
      ptr_map_insert< string_option<CURLOPT_SSLCERT> >(map)("sslcert");
      ptr_map_insert< string_option<CURLOPT_SSLCERTTYPE> >(map)("sslcerttype");
      ptr_map_insert< string_option<CURLOPT_SSLKEY> >(map)("sslkey");
      ptr_map_insert< string_option<CURLOPT_SSLKEYTYPE> >(map)("sslkeytype");
      ptr_map_insert< string_option<CURLOPT_KEYPASSWD> >(map)("keypasswd");
      ptr_map_insert< string_option<CURLOPT_SSLENGINE> >(map)("sslengine");
      ptr_map_insert< string_option<CURLOPT_SSLENGINE_DEFAULT> >(map)("sslengineDefault");
      ptr_map_insert< integer_option<CURLOPT_SSLVERSION> >(map)("sslversion"); // See cURL.SSLVERSION_*
      ptr_map_insert< integer_option<CURLOPT_SSL_VERIFYPEER> >(map)("sslVerifypeer");
      ptr_map_insert< string_option<CURLOPT_CAINFO> >(map)("cainfo");
      ptr_map_insert< string_option<CURLOPT_ISSUERCERT> >(map)("issuercert");
      ptr_map_insert< string_option<CURLOPT_CAPATH> >(map)("capath");
      ptr_map_insert< string_option<CURLOPT_CRLFILE> >(map)("crlfile");
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 1)
      ptr_map_insert< integer_option<CURLOPT_CERTINFO> >(map)("certinfo");
#endif
      ptr_map_insert< string_option<CURLOPT_RANDOM_FILE> >(map)("randomFile");
      ptr_map_insert< string_option<CURLOPT_EGDSOCKET> >(map)("egdsocket");
      ptr_map_insert< integer_option<CURLOPT_SSL_VERIFYHOST> >(map)("sslVerifyhost");
      ptr_map_insert< string_option<CURLOPT_SSL_CIPHER_LIST> >(map)("sslCipherList");
      ptr_map_insert< integer_option<CURLOPT_SSL_SESSIONID_CACHE> >(map)("sslSessionidCache");
      ptr_map_insert< string_option<CURLOPT_KRBLEVEL> >(map)("krblevel");
      // SSH OPTIONS
      ptr_map_insert< integer_option<CURLOPT_SSH_AUTH_TYPES> >(map)("sshAuthTypes"); // See cURL.SSH_AUTH*
      ptr_map_insert< string_option<CURLOPT_SSH_HOST_PUBLIC_KEY_MD5> >(map)("sshHostPublicKeyMd5");
      ptr_map_insert< string_option<CURLOPT_SSH_PUBLIC_KEYFILE> >(map)("sshPublicKeyfile");
      ptr_map_insert< string_option<CURLOPT_SSH_PRIVATE_KEYFILE> >(map)("sshPrivateKeyfile");
      // OTHER OPTIONS
      ptr_map_insert< integer_option<CURLOPT_NEW_FILE_PERMS> >(map)("newFilePerms");
      ptr_map_insert< integer_option<CURLOPT_NEW_DIRECTORY_PERMS> >(map)("newDirectoryPerms");
      // TELNET OPTIONS
      ptr_map_insert< list_option<CURLOPT_TELNETOPTIONS> >(map)("telnetoptions");
      // }END DOC
    }
    return map;
  }

  namespace {
    boost::mutex cookie;

    /* To make sure that curl_global_init are never called from parallel
       threads and are paired cleanly. */
    FLUSSPFERD_CLASS_DESCRIPTION(
        cURL_cookie,
        (constructor_name, "$$cURL_cookie")
        (full_name, "cURL.$$cURL_cookie")
        (constructible, false)
    ) {
    public:
      cURL_cookie(object const &obj)
        : base_type(obj)
        {
          boost::mutex::scoped_lock lock(cookie);
          CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
          if(ret != 0)
            throw flusspferd::exception(std::string("curl_global_init: ")
                                        + curl_easy_strerror(ret));
        }

      ~cURL_cookie() {
        boost::mutex::scoped_lock lock(cookie);
        curl_global_cleanup();
      }
    };
  }
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
