i = new Importer();

i.context = this;

i.load('curl')

c = new cURL();
c.dataReceived = function(blob) {
  this.dataLen += blob.length;
}

c.url = 'http://www.google.co.uk';
c.perform = function() {
  this.dataLen = 0;
  return this.__proto__.perform.call(this);
}
c.perform();
print(c.dataLen);

