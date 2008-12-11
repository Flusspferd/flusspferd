
if (!this.HTTP) this.HTTP = {};

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
          Proxy-Authorization  Range Referer TE User-Agent".split(/\s+/);

  var response_headers = "Accept-Ranges Age ETag Location Proxy-Authenticate \
          Retry-After Server Vary WWW-Authenticate".split(/\s+/);

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

  HTTP.Headers = hdrs; 

})()

