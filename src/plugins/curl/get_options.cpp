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
#include "get_options.hpp"
#include "integer_option.hpp"
#include "string_option.hpp"
#include "list_option.hpp"
#include "function_option.hpp"
#include "http_post_option.hpp"
#include <boost/assign/ptr_map_inserter.hpp>

curl::options_map_t const &curl::get_options() {
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
