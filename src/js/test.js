// vim:ts=2:sw=2:expandtab:autoindent:foldmethod=marker:foldmarker={{{,}}}:
/*
Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

let equiv = require('./test/equiv').equiv;

/**
 * Creates a new Test Suite
 * @constructor
 */
const TestHarness = function Test$Harness(name, options) {

  this.name = name;
  this.tests = [];
  this.asserts_failed = 0;
  this.total_asserts = 0;
  this.tests_failed = 0;
  this.total_tests = 0;
  this.nest = 0;

  for (let [i,k] in Iterator(options || {} )) {
    this[i] = k;
  }

  // Support Flusspferd's IO and the 'system' proposal from ServerJS
  if (!this.outputStream) {
    if (typeof(IO) !== undefined)
      this.outputStream = IO.stdout;
    else {
      const sys = require('system');
      this.outputStream = sys.stdout;
    }
  }

  TestHarness.suites.push(this);
}
exports.Harness = TestHarness; 
TestHarness.suites = [];

TestHarness.go = function TestHarness$go() {
  var suites_failed = [];
  var suites = Array.slice(arguments);
  if (suites.length == 0)
    suites = this.suites;

  if (suites.length == 0)
    return;

  for (var i in suites) {
    var s = suites[i];
    if (s.name === undefined)
      s.name = 'Suite ' + i;
    s.outputStream.print('#', s.name); 
    
    s.go();

    if (s.tests_failed > 0)
      suites_failed.push(s);
  }

  var msg;
  if (suites_failed.length) {
    msg = suites[0].red(suites_failed.length + '/' + suites.length, 'suites failed')
  } else {
    msg = suites[0].green(suites.length + '/' + suites.length, 'suites sucessful');
  }
  suites[0].outputStream.print(msg);
}

TestHarness.prototype = {
  constructor: TestHarness,

  colourize: true,

  same: function() {
    var args = Array.slice(arguments);

    var msg;
    if (args.length > 2)
      msg = args.pop();

    return this.do_assert( {
      type: 'same',
      when: new Date(),
      ok: !!equiv.apply(equiv, args),
      message: msg,
      default_msg: "arguments are the same"
    });
  },

  instanceOf: function(obj, type, msg) {
    var test = obj instanceof type;
    return this.do_assert( {
      type: 'instance_of',
      ok: !!test,
      when: new Date(),
      message: msg,
      default_msg: "object is instance of type"
    });
  },

  ok: function ok(test, msg) {
    if (arguments.length == 0) {
      test = true; // If nothing is passed in, then assume its successful
    }
    return this.do_assert( {
      type: 'ok',
      ok: !!test,
      when: new Date(),
      message: msg,
    } );
  },

  // Handle/print the assert
  do_assert: function do_assert(a) {
    this.current_test.asserts.push(a);
    a.num = this.current_test.asserts.length;

    // Padding to make numbers line up.
    var plan = this.current_test.plan;
    
    if (a.num === 1) {
      // First assert this test
      if (isNaN(plan)) {
        // Just print the newline
        this.outputStream.print();
      }
    }

    if (plan === undefined || isNaN(plan)) 
      plan = a.num;

    let data = [
      a.ok ? this.green('ok') : this.red('not ok'),
      a.num
    ];

    if (a.message !== undefined)
      data.push('-', a.message)
    else if ('default_msg' in a)
      data.push('-', a.default_msg)

    if (a.ok)
      this.current_test.asserts_passed++;
    else 
      this.current_test.asserts_failed++;

    this.outputStream.print(data);
    return a.ok;
  },

  expect: function expect(count) {
    if (this.current_test.asserts.length)
      throw new Error('expect called after an assertion');
    this.current_test.plan = new Number(count).valueOf();
    if (isNaN(this.current_test.plan))
      throw new Error('invalid plan - not a number');
    this.outputStream.print(" 1.." + this.current_test.plan);
  },

  run: function run(name) {
    this.current_test = { 
      name: name, 
      asserts: [],
      asserts_passed: 0,
      asserts_failed: 0,
    };

    if (this[name] instanceof Function === false) {
      throw new Error(name + " is not a function");
    }

    this.tests.push(this.current_test);

    // This writes without a trailing new line
    this.outputStream.write(name + ":");

    // If there's a 'plan' property of the function object, use it as the plan
    if ('expect' in this[name]) {
      this.expect(this[name].expect)
    }

    try {
        this[name].apply(this);
    }
    catch (e) {
      this.current_test.error = e;
    }
    this.finalize_test();
    delete this.current_test;
  },

  go: function go() {
    for (var n in this) {
      if (this.hasOwnProperty(n) == false)
        continue;
      n = n.toString();

      if (n.substr(0,5) === "test_")
        this.run(n);
    }
    this.finalize();
  },

  green: function() {
    var a = Array.prototype.slice.apply(arguments);
    if (a.length == 0 && a[0] instanceof Array)
      a = a[0];

    a = a.join(' ');
    if (!this.colourize || !a.length)
      return a;
    return '\x1b[32m' + a + '\x1b[0m';
  },

  red: function() {
    var a = Array.prototype.slice.apply(arguments);
    if (a.length == 1 && a[0] instanceof Array)
      a = a[0];

    a = a.join(' ');
    if (!this.colourize || !a.length)
      return a;
    return '\x1b[31m' + a + '\x1b[0m';
  },

  finalize_test: function finalize_test() {
    var test = this.current_test;
    var msg = [];
    this.asserts_failed += test.asserts_failed;

    if (test.plan !== null && !isNaN(test.plan)) {
      // Check the plan.
      let plural = test.plan == 1 ? 'assert' : 'asserts';

      if (test.plan > test.asserts.length) {
        // too many
        test.passed = false;
        msg = [test.name, 'expected', test.plan, plural, 'only got', 
               test.asserts.length]; 
        this.total_asserts += test.plan;
        this.asserts_failed += test.plan - test.asserts.length;
      } else if (test.plan < test.asserts.length) {
        test.passed = false;
        msg = [test.name, 'expected', test.plan, plural, 'but got', 
               test.asserts.length]; 
        this.total_asserts += test.asserts.length;
        this.asserts_failed += test.asserts.length - test.plan;
      }
      else {
        test.passed = test.asserts_failed == 0;
        this.total_asserts += test.plan;
      }
    } 

    if (test.error) {
      // The test died.

      // Possibly before it could plan, so add a \n
      if (isNaN(test.plan) && test.asserts.length == 0)
        this.outputStream.print();
      
      let e = this.current_test.error;
      this.outputStream.print( this.red(test.name, 'died:') );
      var e_msg = e.toString().replace(/\n/g, "\n  ");
      let a = ['  ' + e_msg];
      if (e.fileName) {
        var stack = e.stack;
        a.push("\n  ", stack.replace(/\n/g, "\n  "));
      }
      msg = this.red(a);
      test.passed = false;
    } 
    else {
      // No plan, no error.
      test.passed = test.asserts_failed == 0;
    }
    if (msg.length) {
      this.outputStream.print(msg);
    }

    this.total_tests++;
    if (!test.passed)
      this.tests_failed++;
  },

  // Print summary
  finalize: function finalize() {
    if (this.tests_failed == 0) {
      this.outputStream.print(this.green('All tests successful'))
      return;
    }

    var msg = [this.tests_failed + "/" + this.total_tests, "tests",
              "(" + this.asserts_failed + "/" + this.total_asserts,
              "asserts)", "failed"];

    this.outputStream.print(this.red(msg))
  }
}


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
