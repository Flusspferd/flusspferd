// vim:ts=2:sw=2:expandtab:autoindent:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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
    }
  }
  catch (e if /Unable to find library/.exec(e))
  { // Stomp on the module not found message
  }

  cURL.prototype.dataReceived = function (blob) {
    this.responseBlob.append(blob);
  }

  if (cURL.prototype.perform.old === undefined) {
    var old_perform = cURL.prototype.perform;
    cURL.prototype.perform = function perform() {
      this.header_buffer = "";
      delete this.headers;
      delete this.protocol;
      delete this.status;
      delete this.statusMessage;
      this.responseBlob = new Blob(0);
      return perform.old.apply(this, arguments);
    }
    cURL.prototype.perform.old = old_perform;
  }

  return cURL;
})()

/**
 * Class: cURL
 * 
 * Network access via libcurl.
 *
 * Group: Constructor Properties
 *
 * Property: supportedProtocols
 *
 * Array of protocols by this module.
 *
 * Property: versionHex
 *
 * libcurl version number in hex. For example 7.18.2 is 0x071202
 *
 * Property: versionStr
 *
 * libcurl version number in dotted-decimal string form.
 *
 * Group: Constructor
 *
 * Constructor: cURL
 *
 * Construct a cURL object. Takes no parameters
 *
 * Group: Methods
 *
 * Method: perform
 *
 * Method: setMethod
 *
 * Group: Properties
 *
 * Property: url
 *
 * Property: protocol
 *
 * Property: status
 *
 * Property: statusMessage
 *
 * Group: Callback Methods
 *
 * Optional methods that get called when headers or data are received. If the
 * <http.headers> module can be loaded, then <headerReceived> will have a default implementation that will parse the headers into an <HTTP.Headers> object
 *
 * Method: dataReceived
 *
 * Method: headerReceived
 */
