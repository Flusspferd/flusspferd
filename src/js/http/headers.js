// vim:ts=2:sw=2:expandtab:autoindent:
/*
The MIT License

Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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


(function () {

  /* "Good Practice" order of HTTP message headers:
   *    - General-Headers
   *    - Request-Headers
   *    - Response-Headers
   *    - Entity-Headers
   */

  var general_headers = "Cache-Control Connection Date Pragma Trailer \
          Transfer-Encoding Upgrade Via Warning".split(/\s+/); 

  var request_headers = "Accept Accept-Charset Accept-Encoding Accept-Language \
          Authorization Expect From Host If-Match If-Modified-Since \
          If-None-Match If-Range If-Unmodified-Since Max-Forwards \
          Proxy-Authorization  Range Referer TE User-Agent Cookie".split(/\s+/);

  var response_headers = "Accept-Ranges Age ETag Location Proxy-Authenticate \
          Retry-After Server Vary WWW-Authenticate Set-Cookie Set-Cookie2".split(/\s+/);

  var entity_headers = "Allow Content-Encoding Content-Language Content-Length \
          Content-Location Content-MD5 Content-Range Content-Type Expires \
          Last-Modified".split(/\s+/);

  var header_order = [];
  var uc_headers = {};

  for each (let i in [general_headers, request_headers, response_headers, entity_headers]) {
    header_order = header_order.concat(i);
  }

  header_order.forEach(function(x,i) {uc_headers[x.toUpperCase()] = x} );

  // Constructor, take headers (as any case) and assign to proper cassed values
  var hdrs = function HTTP$Headers(hdr_dict) {
    if (hdr_dict === undefined)
      return this;

    for (let [h,v] in Iterator(hdr_dict)) {
      h = h.replace('_', '-'); // CGI environs do this
      h = uc_headers[h.toUpperCase()] || h;
      this[h] = v;
    }
  };

  hdrs.prototype = {
    toString: function() {
      function hdr_val(h,v) {
        if (v instanceof Array)
          return v.map( function(v2) { return hdr_val(h,v2) } );
        else
          return [h + ": " + v];
      }
     
      var lines = [];
      var str = "";
      for each (let h in header_order) {
        if ( !(h in this) )
          continue;

        lines = lines.concat(hdr_val(h,this[h]))
      }
      for (let [h,v] in Iterator(this) ) {
        if ( h.toUpperCase() in uc_headers || !this.hasOwnProperty(h) )
          continue;

        lines = lines.concat(hdr_val(h,this[h]))
      }

      return lines.join("\r\n");
    }
  };

  hdrs.parse = function(str) {
    var headers = {};
    var bytes = 0;
    function get_header_line() {
      var name, tmp, val;
      [tmp,name,val] = /^([^:]+):[ \t]*(.*?)\r\n/.exec(str) || [];
      if (tmp === undefined)
        throw new Error("Malformed HTTP headers after " + bytes + " character(s) : " + str.substr(0,10));

      str = str.substr(tmp.length);
      bytes += tmp.length;
      while (true) {
        // New lines in headers are allowed if followed by a ' ' or a '\t'
        if (str.length == 0 || (str[0] != ' ' && str[0] != '\t'))
          return [name,val];

        let v2="";
        [tmp, v2] = /^[ \t]+(.*?)\r\n/.exec(str) || []
        if (tmp === undefined)
          throw new Error("Malformed HTTP headers: " + str.toSource());

        val += " " + v2;
        str = str.substr(tmp.length);
        bytes += tmp.length;
      }
      return [name,val];

    }

    while (str.length) {
      if (str.match(/^\r\n/)) {
        str = str.substr(2);
      }
      var l = get_header_line();
      if (!l)
        break;
      if (l[0] in headers)
        headers[l[0]] += ", " + l[1];
      else
        headers[l[0]] = l[1];
    }

    if (str.length)
      throw new Error("Malformed HTTP headers");
    return new hdrs(headers);

  }

  // Create setters and getters for other headers, such as contentType sets 
  // Content-Type header
  for each (let h in header_order) {
    h$ = h.split(/-/);
    h$[0] = h$[0].toLowerCase();
    h$ = h$.join("");
    if (h != h$) {
      let k = h;
      hdrs.prototype.__defineGetter__(h$, function() { return this[k] } );
      hdrs.prototype.__defineSetter__(h$, function(v) { return this[k] = v } );
    }
  }

  exports.Headers = hdrs; 

})()

