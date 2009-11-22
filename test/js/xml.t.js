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

function setup() {
  this.doc = xml.DOMParser.parse("test/fixtures/xml/short_1.xml");
}
function teardown() {
  delete this.doc;
  gc();
}

exports.test_DOM = {

  test_nodelist: function() {
    setup.call(this);

    var nl = this.doc.getElementsByTagName('a');
    asserts.instanceOf(nl, xml.NodeList);
    asserts.same(nl.length, 1, "NodeList has a length");
    asserts.instanceOf(nl[0], xml.Node);
    gc();

    var a = nl[0];
    // There was a bug where calling nl[0] a second time would return null;
    asserts.instanceOf(a, xml.Node);
    asserts.ok(a === nl[0], "a === nl[0]");
    gc();

    asserts.instanceOf(a.childNodes, xml.NodeList);
    teardown.call(this);
  },

  test_node_links: function() {
    setup.call(this);

    var a = this.doc.documentElement;
    asserts.ok(a === a.firstChild.parentNode, "a === a.firstChild.parentNode");

    asserts.ok(this.doc === a.ownerDocument, "doc === a.ownerDocument");

    teardown.call(this);
  }
}

if (require.main === module)
  require('test').runner(exports);

