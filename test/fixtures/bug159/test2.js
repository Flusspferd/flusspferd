var sys = require("system");

var print = function(){
  sys.stdout.print.apply(sys.stdout, arguments);
} 

exports.callback = function(){print("running callback");};
