(function() {
  // Load binary module
  $importer.load('curl', true); 

  try {
    // If we have HTTP.Headers, define a default header callback that
    // creates 
    $importer.load('http.headers');
    cURL.prototype.headerReceived = function headerReceived(hdr) {
      if (hdr.match(/^\r\n/)) {
        // Empty line signals end of headers
        try {

          // Pull out HTTP status line.
          var match = /^(HTTP\/1.[10])\s+(\d+)\s+(.*?)\r\n/.exec(this.header_buffer);
          if (match) {
            this.header_buffer = this.header_buffer.substr(match[0].length);
            [undefined, this.protocol, this.status, this.statusMessage] = match;
          }

          this.headers = HTTP.Headers.parse(this.header_buffer);
          delete this.header_buffer;
        }
        catch (e) 
        {
        } // Do nothing on error
      }
      else
        this.header_buffer += hdr;
    },

    cURL.prototype.dataReceived = function (blob) {
      this.responseBlob.append(blob);
    }

    var old_perform = cURL.prototype.perform;
    cURL.prototype.perform = function perform() {
      this.header_buffer = "";
      delete this.headers;
      this.responseBlob = new Blob(0);
      return perform.old.apply(this, arguments);
    }
    cURL.prototype.perform.old = old_perform;

  }
  catch (e if e instanceof ReferenceError)
  { throw e }
  catch (e)
  { print(e) }
})()
