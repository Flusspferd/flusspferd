var test2 = require("test2");

var sys = require("system");

var print = function(){
  sys.stdout.print.apply(sys.stdout, arguments);
}

//var callback = function(){print("running callback");};

gc();

execute(test2.callback);
