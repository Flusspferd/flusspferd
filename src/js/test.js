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
  // TODO: ServerJS compliance
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
    },
  });

  this.paddDepth = 0;
}



merge(TAPProducer.prototype, {
  paddStr: '  ',

  colourize: true,

  green: function green() {
    var a = Array.slice.apply(arguments);
    if (a.length == 0 && a[0] instanceof Array)
      a = a[0];

    a = a.join(' ');
    if (!this.colourize || !a.length)
      return a;
    return '\x1b[32m' + a + '\x1b[0m';
  },

  red: function red() {
    var a = Array.slice.apply(arguments);
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


const Suite = function Suite(cases) {
  this.__proto__.__proto__.constructor.call(this);
  this._state = {
    cases: [],
    casesFailed: 0,
    assertsFailed: [],
    numAsserts: 0,
    run: 0,
    currentCase: undefined
  }

  if (!cases) {
    return this;
  }


  // TODO: setup and teardown methods
  for ([k,t] in Iterator(cases)) {

    if (/^test_/.test(k) == false)
      continue;
    this._state.cases.push({
      func: t,
      name: k.replace(/^test_(.*)/, "$1"),
      index: this._state.cases.length,
      numAsserts: 0,
      assertsFailed: []
    });
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
      for ([name,test] in Iterator(this._state.cases)) {
        this.runCase( test);
      }
    }
    finally {
      currentSuite = oldCurrent;
    }
    this.finalize();
  },

  addSubSuite: function addSubSuite(s) {
  },

  runCase: function runCase(theCase) {
    var asserts = exports.asserts,
        depth = this.paddDepth,
        error;

    this.printPlan(this._state);

    try {
      this.diag(theCase.name + "...");
      this.paddDepth = depth + 1;
      try {
        this._state.currentCase = theCase;
        theCase.func.call(asserts, this);
        this.printPlan(theCase);
      }
      finally {
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
        message: theCase.name,
      });
      return;
    }

    this.do_assert( {
      type: 'case',
      when: new Date(),
      ok: !failed,
      message: theCase.name,
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
    //this.diag(this._state.toSource());
  }
})

exports.Suite = Suite;
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

var currentSuite;
Object.defineProperty( exports, "__currentSuite__", {
  getter: function() { return currentSuite },
  setter: function(x) {
    if (x instanceof Suite)
      return currentSuite = x;
    throw new TypeError("__currentSuite__ must be an instanceof test.Suite");
  },
  configurable: false,
  enumerable: false,
});




const Asserts = function() {
}

merge(Asserts.prototype, {
  same: function() {
    var args = Array.slice(arguments),
        msg;

    if (args.length > 2)
      msg = args.pop();
    var ok = !!equiv.apply(equiv, args);

    var a = {
      type: 'same',
      when: new Date(),
      ok: ok,
      message: msg,
      defaultMsg: "arguments are the same",
    };

    if (!ok) {
      a.wanted = args[0];
      a.got = args.slice(1);
      a.diag = "Wanted: " + (args[0] === undefined
                             ? "undefined"
                             : args[0].toSource())

                          + args.slice(1).map(function(i) {
                              return "\n   Got: "
                                   + (i === undefined
                                          ? "undefined"
                                          : i.toSource()
                                     )
                            });
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
      message: msg,
    } );
  },

  expects: function expects(count) {
    return exports.__currentSuite__.expects(count);
  },

  diag: function diag() {
    var suite = exports.__currentSuite__;
    suite.diag.apply(suite, arguments);
  }

});

exports.asserts = new Asserts();


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
