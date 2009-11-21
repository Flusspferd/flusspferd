const xml = require('xml');
const asserts = require('test').asserts;

exports.test_DomParser = {
  test_simpleOk: function() {
    var doc = xml.DOMParser.parse("test/fixtures/xml/var.xml");
    asserts.instanceOf(doc, xml.Document, "parse returned a Document");
    asserts.instanceOf(doc, xml.Node, "Document isa Node");
  },

  test_simpleFailure: function() {
    asserts.throwsOk( function() {
      xml.DOMParser.parse("test/fixtures/xml/invalid_1.xml");
    } )
  }

}

if (require.main === module)
  require('test').runner(exports);

