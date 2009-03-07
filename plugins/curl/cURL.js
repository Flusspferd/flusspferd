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
  $importer.load('cURL', true); 

  try {
    // If we have HTTP.Headers, define a default header callback that
    // creates 
    $importer.load('HTTP.Headers');
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
 * Synopsis:
 *
 * (code)
 * import('curl')
 * c = new cURL();
 * c.url = 'http://www.google.com';
 * var status = c.perform();
 *
 * // Currently there is no auto-following of redirects, so:
 * while (status >= 300 && status <= 399) {
 *  c.url = c.header.Location;
 *  status = c.perform();
 * }
 * IO.stdout.write(c.responseBlob);
 * (end code)
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
 * Perform the request on <url>.
 *
 * Before the request starts the <headers>, <protocol>, <status>,
 * <statusMessage> and the <responseBlob> properties are cleared
 *
 * Method: setMethod
 *
 * Set the request method to use when <perform> is called. Currently understood
 * values are "GET", "POST", "PUT" and "HEAD".
 *
 * Parameters:
 *  method - request method
 *
 * Group: Properties
 *
 * Property: url
 *
 * The URL to request.
 *
 * Property: protocol
 *
 * HTTP Protocol version. Unkown/undefined for non-HTTP requests. See
 * <headerReceived> for caveats.
 *
 * Property: status
 *
 * Numeric status code. Might not be defined for non-HTTP requests. See
 * <headerReceived> for caveats.
 *
 * Property: statusMessage
 *
 * Human readable status message. Might not be defined for non-HTTP requests.
 * See <headerReceived> for caveats.
 *
 * Property: responseBlob
 *
 * <Blob> of response content. If you replace the <dataReceived> callback then
 * you will have to update this blob yourself.
 *
 * Property: headers
 *
 * <HTTP.Headers> object containing response headers. See <headerReceived> for
 * caveats.
 *
 * Group: Callback Methods
 *
 * Optional methods that get called when headers or data are received. 
 *
 * Method: dataReceived
 *
 * Called when a chunk of body data is available. The default implementation
 * simply appends each chunk of data into <responseBlob>.
 *
 * Parameters:
 *  data - a <Blob> of body data.
 *
 * Method: headerReceived
 *
 * Called for each line of the headers. The line includes the new line
 * characters, and the end of headers are signified by "\r\n".
 *
 * Populates the <headers>, <protocol>, <status> and <statusMessage> properties
 * after all headers have been received.
 *
 * Parameters:
 *  line - a single line of header data
 *
 * This default implementation is conditionally provided on being able to load
 * the <HTTP.Headers> module. If it cannot be loaded then the <headers>,
 * <protocol>, <status> and <statusMessage> properties will not be defined.
 */
