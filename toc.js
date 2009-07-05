function init() {
	var toc = dojo.byId("toc-div");
	toc.innerHTML = "<ul></ul>";
	var ul = dojo.query("ul", toc)[0];

	var headings = dojo.query(".heading");

	make_headings(["h1", "h2", "h3", "h4"], ul, headings, 0, headings.length, "");

	dojo.query("a", ul).removeClass("inthead");
}

function make_headings(types, ul, headings, start, end, old_prefix) {
	var type = types.shift();

	var idx = [];

	for (var i = start; i < end; ++i) {
		var x = headings[i];
		if (x.tagName.toLowerCase() == type)
			idx.push(i);
	}

	idx.push(end);

	for (var i = 0; i < idx.length - 1; ++i) {
		var heading = headings[idx[i]];

		var prefix = old_prefix + (type == "h1" ? "" : ((i + 1) + "."));

		var num = document.createTextNode(prefix.length > 0 ? prefix + " " : "");

		dojo.place(num, heading, "first");

		var li = dojo.create("li", {"innerHTML": heading.innerHTML}, ul);

		if (idx[i+1] - idx[i] > 1) {
			var ul2 = dojo.create("ul", {}, li);
			make_headings(types, ul2, headings, idx[i] + 1, idx[i+1], prefix);
		}
	}
}

dojo.addOnLoad(init);
