const test_const_var = 'A const var at the top level of module a2';

var a2 = require.id;
a2_novar = "a2_novar";

exports.id = require.id;
exports.id_now = function() { return require.id };
exports.array = function() { return [1,2,3] };

exports.var_test = function() { return a2 };

