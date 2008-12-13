// vim:ts=2:sw=2:expandtab:autoindent:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

(function () {

// Test for equality any JavaScript type.
// Discussions and reference: http://philrathe.com/articles/equiv
// Test suites: http://philrathe.com/tests/equiv
// Author: Philippe Rathé <prathe@gmail.com>

// About passing arguments:
//      when < 2   : return true
//      when 2     : return true if 1st equals 2nd
//      when > 2   : return true 1st equals 2nd, and 2nd equals 3rd,
//                      and 3rd equals 4th ... ans so on (by transition)
//

// Stack of constructors that enables us to skip or abort on functions value.
var objectCallerConstructorStack = [];
function equiv() {
    // we will need to use the array splice method,
    // so we need to convert arguments into a true array.
    var args = Array.prototype.slice.apply(arguments);
    var a, b; // compares a and b (1st and 2nd argument)
    var i; // for iterating over objects convenience
    var len; // for iterating array's length memoization
    // Memoized objects' properties.
    // Allow us to test for equivalence only in one way,
    // then to compare properties to make sure no one is missing.
    var aProperties, bProperties;


    if (args.length < 2) {
        return true; // nothing to compare together
    }

    a = args[0];
    b = args[1];

    return (function (a, b) {

        // Try to optimize the algo speed if ever both a and b are references
        // pointing to the same object (e..g. only for functions, arrays and objects)
        if (a === b) {
            return true;
        }
        
        // Must test for the null value before testing for the type of an object,
        // because null is also an object. Trapping null or undefined values here
        // is a good to avoid to much validations further in the code.
        if (a === null || b === null || typeof a === "undefined" || typeof b === "undefined") {
            return false; // Anyway (a === b) would have already caught it.
        }

        // Don't lose time and prevent further wrong type manipulation or unexpected results.
        if (typeof a !== typeof b) {
            return false;
        }

        // Must test if it's an array before testing for the type of an object,
        // because an array is also an object.
        if (a instanceof Array) {

            // Make sure b is also an array.
            // At this point b can be an object.
            // Prevent further arrays operation on object.
            if ( ! (b instanceof Array)) {
                return false;
            }

            len = a.length;
            if (len !== b.length) { // safe and faster
                return false;
            }
            for (i = 0; i < len; i++) {
                if( ! equiv(a[i], b[i])) {
                    return false;
                }
            }
            return true;
        }
        
        // NOTE:
        //      Must test if it's a date before testing if it is an object,
        //      because a date is also an object.
        if (a instanceof Date) {
            return b instanceof Date && a.valueOf() === b.valueOf();
        }

        // NOTE:
        //      Using toSource() to compare regexp only works in FF. It was working as well as the above.
        //      Must verify the constructor of a regexp before verifying it is an object.
        //      Notice those confusing true statements:
        //          var a = /./;
        //          a instanceof Object; // => true
        //          a instanceof RegExp; // => true
        //          typeof a === "function"; // => false in IE and Opera, true in FF and Safari
        //
        // There is 3 possible modifier for regular expressions (g, m and i)
        // The source property only returns the regular expression string without the modifier.
        if (a instanceof RegExp) {
            return b instanceof RegExp &&
                a.source === b.source &&
                a.global === b.global &&
                a.ignoreCase === b.ignoreCase &&
                a.multiline === b.multiline;
        }

        // typeof on some types returns sometimes unexpected type that we deals with.
        // We could have used instanceof instead of typeof but it comes with
        // other problems as mentioned above when testing for the RegExp constructor.
        // We explicitely don't use hasOwnProperty when iterating on the properties of
        // an object to allow a deeper equivalence (e.g. for instances particularly)
        if (typeof a === "object") {
            // Different constructors means a and b can't be equivalent instances.
            // We don't have any choice of comparing them in both ways because of inheritance
            // otherwise {} could be mistaken with []
            // NOTE this:
            //      var a = [];
            //      a instanceof Array;  // true
            //      a instanceof Object; // true
            if ( a.constructor !== b.constructor) {
                return false;
            }

            // Stack constructors before iterating over its properties
            objectCallerConstructorStack.push(a.constructor);

            aProperties = [];
            bProperties = [];

            // NOTE:
            //      We only need to compare propertie's equivalence in one way
            //      and ensures that all properties of a and b are the same.

            // Verify a's properties with b's properties equivalence.
            // Stack a's properties
            
            for (i in a) {
                aProperties.push(i);
                if (!equiv(a[i], b[i])) {
                    // Unstack current constructor when finished with the object
                    objectCallerConstructorStack.pop();
                    return false;
                }
            }

            // Stack also b's properties
            for (i in b) {
                bProperties.push(i);
            }

            // Unstack current constructor when finished with the object
            objectCallerConstructorStack.pop();

            // Ensures properties's name in both ways.
            // Must sort them because they may not be always in the same order!
            return equiv(aProperties.sort(), bProperties.sort());
        }

        // NOTE:
        //      Using the typeof operator will also catch a RegExp instance.
        //      Take no risk.
        if (a instanceof Function) {
            // BEHAVIOR when comparing functions.
            //      Being here means that a and b were were not the same references.
            //      At this level, functions won't be compared at all.
            //      - skip when they are properties of instances (not instantiates from the Object construtor)
            //      - abort otherwise
            //
            //      To Determine if a function is anonymous we use a regexp on the function sources.
            var currentObjectCallerConstructor =
                    objectCallerConstructorStack[objectCallerConstructorStack.length - 1];
            return currentObjectCallerConstructor !== Object &&
                    typeof currentObjectCallerConstructor !== "undefined";
        }

        // NaN is a number in JavaScript.
        // Because this statement is false: 0/0 === 0/0,
        // we must use the isNaN function which is the only way to know a number is NaN.
        // Note that it isn't the case with the Infinity number, and that 1/0 === 2/0.
        if (typeof a === "number" && isNaN(a)) {
            return isNaN(b);
        }

        // Compares Number, String or Boolean
        // Because sooner we have already tried:
        //          a === b (then return if it is the case)
        //            and
        //  typeof a !== typeof b (then return false if it is the case)
        // at this time a and b should be of the same type, but hold different values.
        // We can safely return false instead of returning a === b
        return false;

    }(a, b)) && equiv.apply(this, args.splice(1, args.length -1)); // apply transition with (1..n) arguments
}

this.TestHarness = function TestHarness() {
  this.tests = [];
  this.asserts_failed = 0;
  this.total_asserts = 0;
  this.tests_failed = 0;
  this.total_tests = 0;
  this.outputStream = IO.stdout;
}

this.TestHarness.prototype = {
  constructor: TestHarness,

  colourize: true,

  same: function() {
    var args = Array.prototype.slice.call(arguments);

    var msg;
    if (args.length > 2)
      msg = args.pop();

    this.do_assert( {
      type: 'same',
      when: new Date(),
      ok: !!equiv.apply(equiv, args),
      message: msg,
      default_msg: "arguments are the same"
    });
  },

  instance_of: function(obj, type, msg) {
    var test = obj instanceof type;
    this.do_assert( {
      type: 'instance_of',
      ok: !!test,
      when: new Date(),
      message: msg,
      default_msg: "object is instance of type"
    });
  },

  ok: function ok(test, msg) {
    if (arguments.length == 0) {
      test = true; // If nothing is passed, then assume its successful
    }
    this.do_assert( {
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
    var padd = "  ";
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

    let len = plan.toString().length - a.num.toString().length;
    if (len > 0)
      padd += Array(len+1).join(' ');

    let data = [padd + a.num];
    if (this.colourize)
      data.push(a.ok ? '\x1b[32mok\x1b[0m' : '\x1b[31mnot ok\x1b[0m');
    else
      data.push(a.ok ? 'ok' : 'not ok');

    if (a.message !== undefined)
      data.push('-', a.message)
    else if ('default_msg' in a)
      data.push('-', a.default_msg)

    if (a.ok)
      this.current_test.asserts_passed++;
    else 
      this.current_test.asserts_failed++;

    this.outputStream.print(data);
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
      this.outputStream.print(test.name, 'died:');
      var e_msg = e.toString().replace(/\n/g, "\n  ");
      let a = ['  ' + e_msg];
      if (e.fileName) {
        a.push('at', e.fileName + ':' + e.lineNumber);
      }
      msg = a;
      test.passed = false;
    } 
    else {
      // No plan, no error.
      test.passed = test.asserts_failed == 0;
    }
    if (msg.length)
      this.outputStream.print(msg);

    this.total_tests++;
    if (!test.passed)
      this.tests_failed++;
  },

  // Print summary
  finalize: function finalize() {
    if (this.tests_failed == 0) {
      var msg = 'All tests successful';
      if (this.colourize)
        msg = '\x1b[32m' + msg + '\x1b[0m';
      this.outputStream.print(msg)
      return;
    }

    var msg = [this.tests_failed + "/" + this.total_tests, "tests",
              "(" + this.asserts_failed + "/" + this.total_asserts,
              "asserts)", "failed"];
    if (this.colourize) {
      msg[0] = '\x1b[31m' + msg[0];
      msg[msg.length-1] = msg[msg.length-1] + '\x1b[0m';
    }


    this.outputStream.print(msg)
  }
}


})()

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
  [this.ok(1, i) for (i in Range(0,12) )];
}

t.test_gt = function() {
  this.expect(1);
  this.ok();
  this.ok();
  this.ok();
}

t.go();
*/
