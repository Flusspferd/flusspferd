const xml = require('xml');
const asserts = require('test').asserts;

// A few simple tests to make sure the DOMParser class is working right
exports.test_DOMParser = {
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

exports.test_DOM = {
  test_nodelist: function() {
    var doc = xml.DOMParser.parse("test/fixtures/xml/short_1.xml");

    var nl = doc.getElementsByTagName('a');
    asserts.instanceOf(nl, xml.NodeList);
    asserts.same(nl.length, 1, "NodeList has a length");
    asserts.instanceOf(nl[0], xml.Node);
    gc();

    // There was a bug where calling nl[0] a second time would return null;
    asserts.instanceOf(nl[0], xml.Node);
    asserts.ok(nl[0] === nl[0], "nl[0] === nl[0]");
  }
}

if (require.main === module)
  require('test').runner(exports);

