

exports.test_deep_recrusion = function() {
 
  var obj = {
    get_caller: function() { },
    deprecation_warning: function () {
        var stack = obj.getCaller(2);
    }
  };

  Object.defineProperty(obj, "getCaller", {
    enumerable: false,
    getter: function() { 
      obj.deprecation_warning();
      return obj[ "get_caller" ]; 
    }
  });


  require('test').asserts.throwsOk(
    function() { obj.getCaller(1) },
    "InternalError: too much recursion",
    "recursion error throw"
  );
}

if (require.main === module)
  require('test').runner(exports);
