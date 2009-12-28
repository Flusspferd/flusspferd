// http://wiki.commonjs.org/wiki/Unit_Testing/1.0
//
// Originally from narwhal.js (http://narwhaljs.org)
// Copyright (c) 2009 Thomas Robinson <280north.com>
// Tweaks added by Ash Berlin, Flusspferd project, 2009
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

var pSlice = Array.prototype.slice,
    has_error_instance_stack = (new Error()).stack,
    equiv = require('./test/equiv').equiv;

// 1. The assert module provides functions that throw
// AssertionError's when particular conditions are not met. The
// assert module must conform to the following interface.

var assert = exports;

// 2. The AssertionError is defined in assert.
// new assert.AssertionError({message: message, actual: actual, expected: expected})

assert.AssertionError = function AssertionError(options) {
    if (typeof options != "object")
      throw new TypeError("assert.AssertionError requires an object");

    this.name = "AssertionError";
    this.message = options.message;
    this.actual = options.actual;
    this.expected = options.expected;
    this.operator = options.operator;

    if ("stack" in options) {
      this.stack = options.stack;
      if ("lineNumber" in options) this.lineNumber = options.lineNumber;
      if ("fileName" in options) this.fileName = options.fileName;
    }
    //v8 specific
    else if(Error.captureStackTrace){
        Error.captureStackTrace(this,fail);
        //node specific, removes the node machinery stack frames
        if(typeof(__filename) !== undefined){
            var stack = this.stack.split('\n');
            for (var i = stack.length - 1; i >= 0; i--) {
                if(stack[i].indexOf(__filename) != -1){
                    this.stack = stack.slice(0,i+2).join('\n');
                    break;
                }
            }
        }
    }
    // spidermonkey specific
    else if (has_error_instance_stack){
        // splice(1) removes the Error() ctor from the stack
        var stack = (new Error).stack.split("\n").slice(1); 
        
        var p = /^.*?@(.*?):(.*?)$/.exec( stack[1] ); 
        this.fileName = p[1];
        this.lineNumber = p[2];
        this.stack = stack.join("\n");
    }
    else if (typeof Packages != "undefined") {
        this.rhinoException = Packages.org.mozilla.javascript.JavaScriptException(this, null, 0);
    }
};

assert.AssertionError.prototype.toString = function(){
    if(this.message)
        return [this.name+":", this.message].join(" ");
    else {
        return [
          this.name+":", 
          uneval(this.expected ), 
          this.operator,
          uneval(this.actual)
        ].join(" ");
    }
}

assert.AssertionError.prototype.toSource = function(){
  return "new (require('assert').AssertionError)" + Object.prototype.toSource.call(this)
}

// assert.AssertionError instanceof Error

assert.AssertionError.prototype.__proto__ = Error.prototype;

// At present only the three keys mentioned above are used and
// understood by the spec. Implementations or sub modules can pass
// other keys to the AssertionError's constructor - they will be
// ignored.

// 3. All of the following functions must throw an AssertionError
// when a corresponding condition is not met, with a message that
// may be undefined if not provided.  All assertion methods provide
// both the actual and expected values to the assertion error for
// display purposes.

assert.fail = function (options) {
    throw new assert.AssertionError(options);
};

// XXX extension
assert.pass = function () {
};

// XXX extension
assert.error = function () {
};


// 4. Pure assertion tests whether a value is truthy, as determined
// by !!guard.
// assert.ok(guard, message_opt);
// This statement is equivalent to assert.equal(true, guard,
// message_opt);. To test strictly for the value true, use
// assert.strictEqual(true, guard, message_opt);.

assert.ok = function (value, message) {
    if (!!!value)
        (this.fail || assert.fail)({
            "actual": value,
            "expected": true,
            "message": message,
            "operator": "=="
        });
    else
        (this.pass || assert.pass)(message);
};

// 5. The equality assertion tests shallow, coercive equality with
// ==.
// assert.equal(actual, expected, message_opt);

assert.equal = function (actual, expected, message) {
    if (actual != expected)
        (this.fail || assert.fail)({
            "actual": actual,
            "expected": expected,
            "message": message,
            "operator": "=="
        });
    else
        (this.pass || assert.pass)(message);
};


// 6. The non-equality assertion tests for whether two objects are not equal
// with != assert.notEqual(actual, expected, message_opt);

assert.notEqual = function (actual, expected, message) {
    if (actual == expected)
        (this.fail || assert.fail)({
            "actual": actual,
            "expected": expected,
            "message": message,
            "operator": "!="
        });
    else
        (this.pass || assert.pass)(message);
};

// 7. The equivalence assertion tests a deep equality relation.
// assert.deepEqual(actual, expected, message_opt);

exports.deepEqual = function deepEqual(actual, expected, message) {
    if (!_deepEqual(actual, expected))
        (this.fail || assert.fail)({
            "actual": actual,
            "expected": expected,
            "message": message, 
            "operator": "notDeepEqual"
        });
    else
        (this.pass || assert.pass)(message);
};

function _deepEqual(actual, expected) {
    // 7.1. All identical values are equivalent, as determined by ===.
    if (actual === expected) {
        return true;

    // 7.2. If the expected value is a Date object, the actual value is
    // equivalent if it is also a Date object that refers to the same time.
    } else if (actual instanceof Date
            && expected instanceof Date) {
        return actual.getTime() === expected.getTime();

    // 7.3. Other pairs that do not both pass typeof value == "object",
    // equivalence is determined by ==.
    } else if (typeof actual != 'object'
            && typeof expected != 'object') {
        return actual == expected;

    // 7.4. For all other Object pairs, including Array objects, equivalence is
    // determined by having the same number of owned properties (as verified
    // with Object.prototype.hasOwnProperty.call), the same set of keys
    // (although not necessarily the same order), equivalent values for every
    // corresponding key, and an identical "prototype" property. Note: this
    // accounts for both named and indexed properties on Arrays.
    } else {
        return objEquiv(actual, expected);
    }
}

function isUndefinedOrNull (value) {
    return value === null || value === undefined;
}

function isArguments (object){
    return Object.prototype.toString.call(object) == '[object Arguments]';
}

function objEquiv (a, b) {
    if (isUndefinedOrNull(a) || isUndefinedOrNull(b))
        return false;
    // an identical "prototype" property.
    if (a.prototype !== b.prototype) return false;
    //~~~I've managed to break Object.keys through screwy arguments passing.
    //   Converting to array solves the problem.
    if(isArguments(a)){
        if(!isArguments(b)){
            return false;
        }
        a = pSlice.call(a);
        b = pSlice.call(b);
        return _deepEqual(a,b);
    }
    try{
        var ka = Object.keys(a),
            kb = Object.keys(b),
            key, i;
    } catch (e) {//happens when one is a string literal and the other isn't
        return false;
    }
    // having the same number of owned properties (keys incorporates hasOwnProperty)
    if(ka.length != kb.length)
        return false;
    //the same set of keys (although not necessarily the same order),
    ka.sort();
    kb.sort();
    //~~~cheap key test
    for (i = ka.length - 1; i >= 0; i--) {
        if(ka[i] != kb[i])
            return false;
    }
    //equivalent values for every corresponding key, and
    //~~~possibly expensive deep test
    for (i = ka.length - 1; i >= 0; i--) {
        key = ka[i];
        if(!_deepEqual(a[key], b[key] ))
           return false;
    }
    return true;
}

// 8. The non-equivalence assertion tests for any deep inequality.
// assert.notDeepEqual(actual, expected, message_opt);

exports.notDeepEqual = function notDeepEqual(actual, expected, message) {
    if (_deepEqual(actual, expected))
        (this.fail || assert.fail)({
            "actual": actual,
            "expected": expected,
            "message": message, 
            "operator": "notDeepEqual"
        });
    else
        (this.pass || assert.pass)(message);
};


// 9. The strict equality assertion tests strict equality, as determined by ===.
// assert.strictEqual(actual, expected, message_opt);

assert.strictEqual = function (actual, expected, message) {
    if (actual !== expected)
        (this.fail || assert.fail)({
            "actual": actual,
            "expected": expected,
            "message": message,
            "operator": "==="
        });
    else
        (this.pass || assert.pass)(message);
};

// 10. The strict non-equality assertion tests for strict inequality, as determined by !==.
// assert.notStrictEqual(actual, expected, message_opt);

assert.notStrictEqual = function (actual, expected, message) {
    if (actual === expected)
        (this.fail || assert.fail)({
            "actual": actual,
            "expected": expected,
            "message": message,
            "operator": "!=="
        });
    else
        (this.pass || assert.pass)(message);
};

// 11. Expected to throw an error:
// assert.throws(block, Error_opt, message_opt);

assert["throws"] = function (block, ErrorClass, message) {
    var threw = false,
        exception = null;

    // (block)
    // (block, message:String)
    // (block, Error)
    // (block, Error, message)

    if (typeof ErrorClass == "string") {
        message = ErrorClass;
        Error = undefined;
    }

    try {
        block();
    } catch (e) {
        threw = true;
        exception = e;
    }
    
    if (!threw) {
        (this.fail || assert.fail)({
            "message": message,
            "operator": "throw"
        });
    } else if (ErrorClass) {
        if (exception instanceof ErrorClass)
            (this.pass || assert.pass)(message);
        else
            throw exception;
    } else {
        (this.pass || assert.pass)(message);
    }

};

