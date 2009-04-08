/*
  Based on the following work.

  @author: remy sharp / http://remysharp.com
  @url: http://remysharp.com/2008/04/01/wiki-to-html-using-javascript/
  @license: Creative Commons License - ShareAlike http://creativecommons.org/licenses/by-sa/3.0/
  @version: 1.0
  
  Can extend String or be used stand alone - just change the flag at the top of the script.
*/

(function () {
    
var extendString = true;

if (extendString) {
    String.prototype.wiki2html = wiki2html;
    String.prototype.iswiki = iswiki;
} else {
    window.wiki2html = wiki2html;
    window.iswiki = iswiki;
}

// utility function to check whether it's worth running through the wiki2html
function iswiki(s) {
    if (extendString) {
        s = this;
    }

    return !!(s.match(/^[\s{2} `#\*='{2}]/m));
}

function inlinewiki(s) {
  // TODO: This should probably be a single regexp with multiple captures so
  // that things like "foo ** bar //baz ** fnord//" dont produce invalid markup
  return s
    .replace(/\*{2}(.*?)\*{2}/g, function (m, l) { // **bold**
        return '<strong>' + l + '</strong>';
    })

    .replace(/\/{2}(.*?)\/{2}/g, function (m, l) { // //italic//
        return '<em>' + l + '</em>';
    })

    .replace(/`(.*?)`/g, function (m, l) { // `code`
        return '<code>' + l + '</code>';
    })

    // TODO: Decipher these links and work out their syntax
    .replace(/[^\[](http[^\[\s]*)/g, function (m, l) { // normal link
        return '<a href="' + l + '">' + l + '</a>';
    })

    .replace(/[\[](http.*)[!\]]/g, function (m, l) { // external link
        var p = l.replace(/[\[\]]/g, '').split(/ /);
        var link = p.shift();
        return '<a href="' + link + '">' + (p.length ? p.join(' ') : link) + '</a>';
    })

    .replace(/\[\[(.*?)\]\]/mg, function (m, l) { // internal link or image
        var p = l.split(/\|/);
        var link = p.shift();

        if (link.match(/^Image:(.*)/)) {
            // no support for images - since it looks up the source from the wiki db :-(
            return m;
        } else {
            return new Link().toSymbol(link).withText((p.length ? p.join('|') : link));
        }
    })
}


// the regex beast...
function wiki2html(s) {
    if (extendString) {
        s = this.toString();
    }
    
    // lists need to be done using a function to allow for recusive calls
    function list(str) {
        return str.replace(/(?:(?:(?:^|\n)[\*#].*)+)/g, function (m) {  // (?=[\*#])
            var type = m.match(/(^|\n)#/) ? 'OL' : 'UL';
            // strip first layer of list
            m = m.replace(/(^|\n)[\*#][ ]{0,1}/g, "$1");
            m = list(m);
            return '<' + type + '><li>' + m.replace(/^\n/, '').split(/\n/).join('</li><li>') + '</li></' + type + '>';
        });
    }

    var out = '';

    var s = s + "\n\n";

    while (s.length) {
        s = s.replace(/^(?:\s*?\n)+/, '');
        var matched = false;

        // ==headings==
        s = s.replace(/^([=]+)([^=]*)\1\n/g, function (m, l, t) { 
            matched = true;
            out += '<h' + l.length + '>' + t + '</h' + l.length + '>\n';
            return '';
        })
        if (matched) continue;

        /// {{{ code sample }}}
        s = s.replace(/^{{{((?:\n|.)*?)}}}\s*\n/, function(m,c) {
            out += '<pre class="code">' + c + '</pre>';
            matched = true;
            return '';
        });
        if (matched) continue;

        s = s.replace(/^([*#])(\s+)(.*)(?:\n \1(.*))*/, function(m, type, indent) {
            matched = true;
            return '';
        });
        if (matched) continue;

        // paragraphs\n\n
        s = s.replace(/^(.*?(?:\n.*?)*?)\n\n/, function (m, l) {
            matched = true;
            out += "<p>" + inlinewiki(l) + "</p>\n";
            return '';
        })
        if (matched) continue;

        break; // Dont loop if we can't work out what it should be.
    }
    return out + s;
}
    
})();

