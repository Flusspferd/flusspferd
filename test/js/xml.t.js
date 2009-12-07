const xml = require('xml');
const asserts = require('test').asserts;

// A few simple tests to make sure the XMLParser class is working right
exports.test_XMLParser = {
  test_simpleOk: function() {
    var doc = xml.XMLParser.parse("test/fixtures/xml/var.xml");
    asserts.instanceOf(doc, xml.Document, "parse returned a Document");
    asserts.instanceOf(doc, xml.Node, "Document isa Node");
  },

  test_simpleFailure: function() {
    asserts.throwsOk( function() {
      xml.XMLParser.parse("test/fixtures/xml/invalid_1.xml");
    } )
  }

}

function setup(file) {
  file = file || "short_1.xml";
  this.doc = xml.XMLParser.parse("test/fixtures/xml/" + file );
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
    asserts.instanceOf(a, xml.Element);
    var b = a.firstChild;

    asserts.ok(a === b.parentNode, "a === a.firstChild.parentNode");

    asserts.ok(this.doc === a.ownerDocument, "doc === a.ownerDocument");

    teardown.call(this);
  },

  test_node_methods: function() {
    setup.call(this);

    var a = this.doc.documentElement;

    var b = a.cloneNode();
    asserts.ok(b !== a, "b !== a");
    asserts.same(b.hasChildNodes(), false, "cloned node has no children");
    asserts.same(b.parentNode, null, "cloned node has no parent");
    asserts.same(b.ownerDocument, this.doc, "cloned node has right ownerDocument");
    gc();

    var c = a.cloneNode(true);
    asserts.ok(c !== a, "c !== a");
    asserts.ok(c !== b, "c !== b");
    asserts.same(c.hasChildNodes(), a.hasChildNodes(), "cloned node has children");
    asserts.same(c.parentNode, null, "cloned node has no parent");
    asserts.same(c.ownerDocument, this.doc, "cloned node has right ownerDocument");

    teardown.call(this);
  },

  test_named_node_map: function() {
    setup.call(this, "attr_1.xml");

    var map = this.doc.documentElement.attributes;

    asserts.instanceOf(map, xml.NamedNodeMap);
    asserts.same(map.length, 2, "correct lenght")

    var i0, foo;
    asserts.ok(i0 = map.item(0), "can get an item");
    asserts.ok(foo = map.getNamedItem("foo"), "can get a named item");
    asserts.ok(i0 === foo, "same object via name and number");
    asserts.instanceOf(foo, xml.Attr);
    asserts.same(foo.name, "foo", "right name");
    asserts.same(foo.value, "bar", "right value");

    teardown.call(this);
  },

  test_char_nodes: function() {
    setup.call(this);

    var n = this.doc.documentElement.firstChild;

    asserts.instanceOf(n, xml.Text);
    asserts.same(n.nodeType, xml.Node.TEXT_NODE);
    asserts.same(n.data, "\n  content\n  ");

    teardown.call(this);
  }
}

exports.test_HTML = function() {
  var doc = xml.HTMLParser.parse("test/fixtures/xml/sample.html");
  asserts.instanceOf(doc, xml.Document, "parse returned a Document");
  asserts.instanceOf(doc, xml.Node, "Document isa Node");
  asserts.same(String(doc), '<?xml version="1.0"?>\n<html xmlns="http://www.w3.org/1999/xhtml"><body>\n<p>foo <b>baz <i>quxx</i></b><i> flibble</i>\n</p><p>\n</p></body></html>', "XML output as expected");
}

exports.test_DOMException = function() {
  setup.call(this);

  var err, doc = this.doc;

  // Cant use throwsOk cos we want the error
  try {
    doc.insertBefore(doc.documentElement, doc.documentElement);
    asserts.ok(false, "DOMException thrown");
  }
  catch (e) {
    err = e;
  };
  asserts.instanceOf(err, xml.DOMException);

  // const unsigned short      HIERARCHY_REQUEST_ERR          = 3;
  asserts.same(err.code, 3, "err.code == HIERARCHY_REQUEST_ERR");

  teardown.call(this);
}

if (require.main === module)
  require('test').runner(exports);

