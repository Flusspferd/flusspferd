// vim:ts=2:sw=2:expandtab:autoindent:foldmethod=marker:foldmarker={{{,}}}:
/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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

let {equiv:equiv} = require('./test/equiv');
let Util = require('util');

function merge(ctor, obj) {
  for (let [k,v] in Iterator(obj))
    ctor[k] = v;
}

const TAPProducer = function TAPProducer() {
  this.outputStream = require('system').stdout;

  var depth;
  Object.defineProperty(this, "paddDepth", {

    getter: function() { return depth },
    setter: function(d) {
      d = new Number(d).valueOf();
      if (isNaN(d))
        throw TypeError("paddDepth must be a number");
      depth = d;
      this.padding = this.getPadd();
    }
  });

  this.paddDepth = 0;
}



merge(TAPProducer.prototype, {
  paddStr: '  ',

  colourize: true,

  green: function green() {
    var a = Array.slice(arguments);
    if (a.length == 0 && a[0] instanceof Array)
      a = a[0];

    a = a.join(' ');
    if (!this.colourize || !a.length)
      return a;
    return '\x1b[32m' + a + '\x1b[0m';
  },

  red: function red() {
    var a = Array.slice(arguments);
    if (a.length == 1 && a[0] instanceof Array)
      a = a[0];

    a = a.join(' ');
    if (!this.colourize || !a.length)
      return a;
    return '\x1b[31m' + a + '\x1b[0m';
  },

  // Handle/print the assert
  do_assert: function do_assert(a) {
    
    a.num = ++this._state.currentCase.numAsserts;

    let data = [a.num];
    data.push(a.ok ? this.green('ok') : this.red('not ok'));

    if (a.message !== undefined)
      data.push('-', a.message)
    else if ('defaultMsg' in a)
      data.push('-', a.defaultMsg)

    if (!a.ok)
      this._state.currentCase.assertsFailed.push(a);

    this.print(data);

    if (a.diag)
      this.diag(a.diag);
    if (a.errorDiag)
      this.diag(a.errorDiag, this.red);
    return a.ok;
  },

  print: function print() {
    var msg;
    if (arguments[0] instanceof Array)
      msg = arguments[0].join(" ");
    else
      msg = Array.join(arguments, " ");

    this.outputStream.print(this.padding + msg)
  },

  diag: function diag(msg,filter) {
    var self = this,
        lines = msg.split(/\n/)

    lines = lines.map(function(l) {
      return self.padding + ("# " + l)
    });

    if (filter)
      lines = lines.map(function(x) { return filter.call(self,x) });
    lines = lines.join("\n");

    this.outputStream.print(lines);
  },

  getPadd: function getPadd() {
    var i = 0;
    return [this.paddStr for (i in Util.Range(0, this.paddDepth))].join('');
  }
});


const Suite = exports.Suite = function Suite(cases) {
  this.__proto__.__proto__.constructor.call(this);
  this._state = {
    cases: [],
    assertsFailed: [],
    numAsserts: 0,
    currentCase: undefined
  }
  this.nested = false;

  if (!cases) {
    return;
  }


  // TODO: setup and teardown methods
  for (let [k,t] in Iterator(cases)) {

    if (/^test_/.test(k) == false)
      continue;

    let testCase = {
      name: k.replace(/^test_(.*)/, "$1"),
      index: this._state.cases.length,
      numAsserts: 0,
      assertsFailed: []
    };

    // Not a function - must be a sub-test
    if (typeof t != "function") {
      let s = testCase.suite = new Suite(t);
      s.nested = true;
      s.name = k;
    }
    else testCase.func = t;

    this._state.cases.push(testCase);
  }

  this._state.plan = this._state.cases.length;
}

Suite.prototype.__proto__ = TAPProducer.prototype;

merge(Suite.prototype, {

  run: function run() {
    if (!this._state.cases.length) {
      throw new Error("Cannot run a Test.Suite without any test cases!");
    }

    var oldCurrent = exports.__currentSuite__;
    try {
      exports.__currentSuite__ = this;
      for (let [name,test] in Iterator(this._state.cases)) {
        this.runCase( test);
      }
    }
    finally {
      exports.__currentSuite__ = oldCurrent;
    }
    this.finalize();
  },

  runCase: function runCase(theCase) {
    var asserts = exports.asserts,
        depth = this.paddDepth,
        error;

    try {
      this.diag(theCase.name + "...");
      this.paddDepth = depth + 1;
      try {
        this._state.currentCase = theCase;
        if (theCase.suite) {
          var nested = theCase.suite;
          nested.paddDepth = this.paddDepth;

          nested.run();
          theCase = nested._state;
        }
        else {
          theCase.func.call(asserts, this);
        }
      }
      finally {
        this.printPlan(theCase);
        this.paddDepth = depth;
      }
    }
    catch (e) {
      error = e;
    }
    this.finishCase(theCase, error)
  },

  finishCase: function finishCase(theCase, error) {
    var failed = error || theCase.assertsFailed.length > 0;
    this._state.currentCase = this._state;
    if (error) {

      var diag = "case terminated with exception:\n"
               + error.toString();

      if (error.fileName) {
        diag += "\n  " +  error.stack.replace(/\n/g, "\n  ");
      }

      this.do_assert( {
        type: 'case',
        when: new Date(),
        errorDiag: diag,
        ok: !failed,
        message: theCase.name
      });
      return;
    }

    this.do_assert( {
      type: 'case',
      when: new Date(),
      ok: !failed,
      message: theCase.name
    });
    delete this._state.currentCase;

  },

  printPlan: function printPlan(theCase) {
    if (theCase.planned)
      return;

    if ("plan" in theCase == false)
      theCase.plan = theCase.numAsserts;

    this.print("1.." + theCase.plan);
    theCase.planned = true;
  },

  finalize: function finalize() {
    if (this.nested)
      return;
    if (this._state.assertsFailed.length)
      this.print(this.red("Failed", this._state.assertsFailed.length, "tests"));
    else
      this.print(this.green("All tests successful"));
  }
})

exports.runner = function runner() {
  var suite;
  var args = Array.slice(arguments);
  if (args.length == 1 && args[0] instanceof Suite) {
    builder = args[0];
  }
  else if (args.every(function(x) { return x instanceof Suite })) {
    suite = new Suite();
    args.forEach(suite.addSubSuite, builder);
  }
  else if (args[0] instanceof Object) {
    suite = new Suite(args[0]);
  }
  else {
    throw new TypeError("Invalid arguments to test.runner");
  }

  return suite.run();
};

// The general test runner for 'run this dir of tests'
exports.prove = function prove() {
  const fs = require('filesystem-base');

  var i = 0;
  var options = {};

  if (typeof arguments[0] == "object") {
    i++;
    options = arguments[0];
  }

  var test_files = [];

  for (; i < arguments.length; ++i) {
    var x = arguments[i];
    if (!fs.exists(x))
      throw new TypeError("Cannot determine test source for " + uneval(x));

    if (fs.isDirectory(x))
      findTests(fs.list(x));
    else
      findTests([x]);
  }

  var test = loadTests(test_files);
  var suite = new Suite(test);
  suite.run();

  var failed = suite._state.assertsFailed.length;
  quit( failed <= 255 ? failed : 255 );

  function findTests(files) {
    for ([,x] in Iterator(files) ) {
      // Ingore "." dot-files
      if (x.match(/(?:^|[\/\\])\.[^\/]*$/))
        continue;

      if (fs.isDirectory(x)) {
        if (options.recurse)
          findTests(fs.list(x));
        continue;
      }

      if (x.match(/\.t\.js$/))
        test_files.push(x);
    }
  }

  function loadTests(files) {
    var filename;
    var testObject = {};
    for ([,filename] in Iterator(files) ) {
      var id  = fs.canonical(filename);

      var test = require('file://' + id);
      testObject['test_' + id] = test;
    }

    return testObject;
  }
}

var currentSuite;
Object.defineProperty( exports, "__currentSuite__", {
  getter: function() { return currentSuite },
  setter: function(x) {
    if (x === undefined || x instanceof Suite)
      return currentSuite = x;
    throw new TypeError("__currentSuite__ must be an instanceof test.Suite");
  },
  configurable: false,
  enumerable: false
});



exports.asserts = {
  same: function(got, expected, message) {

    var ok = !!equiv(got, expected);

    var a = {
      type: 'same',
      when: new Date(),
      ok: ok,
      message: message,
      defaultMsg: "arguments are the same"
    };

    if (!ok) {
      a.diag = "   Got: " + (got === undefined
                             ? "undefined"
                             : uneval(got))
             + "\nWanted: " + (expected === undefined
                             ? "undefined"
                             : uneval(expected));
    }
    return exports.__currentSuite__.do_assert(a);
  },

  instanceOf: function(obj, type, msg) {
    var test = obj instanceof type;
    return exports.__currentSuite__.do_assert( {
      type: 'instanceof',
      ok: !!test,
      when: new Date(),
      message: msg,
      defaultMsg: "is instanceof "
                  + (type.name || "correct type")
    });
  },

  ok: function ok(test, msg) {
    if (arguments.length == 0) {
      test = true; // If nothing is passed in, then assume its successful
    }
    return exports.__currentSuite__.do_assert( {
      type: 'ok',
      ok: !!test,
      when: new Date(),
      message: msg
    } );
  },

  matches: function matches(value, re, msg) {
    if (re instanceof RegExp == false) {
      re = new RegExp(re);
    }
    var ok = !!re(value);

    var a = {
      type: 'matches',
      ok: ok,
      when: new Date(),
      message: msg,
      defaultMsg: "matches " + re,
    };

    if (!ok) {
      a.diag = (value === undefined
                ? "undefined"
                : uneval(value))
             + "\nDidn't Match: " + re
    }

    return exports.__currentSuite__.do_assert(a);
  },

  diag: function diag() {
    var suite = exports.__currentSuite__;
    suite.diag.apply(suite, arguments);
  }

};


/*
t = new TestHarness;
t.test_expect_prop = function() {
  with(this) {
    ok(true, "foo")
  }
}
t.test_expect_prop.expect = 6;

t.test_die = function() {
  throw new Error("foo\nbar");
}

t.test_noplan = function() {
  this.ok("no plan");
  this.same(1,new Number("1"), "numbers are the same");
}

t.test_gt_ten = function () {
  this.expect(12);
  [this.ok(1, i) for (i in Util.Range(0,12) )];
}

t.test_gt = function() {
  this.expect(1);
  this.ok();
  this.ok();
  this.ok();
}

t.go();
*/
