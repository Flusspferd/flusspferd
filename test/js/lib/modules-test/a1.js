const test_const_var = 'A const var at the top level of module a1';

exports.id = require.id;
exports.id_now = function() { return require.id };

exports.a2 = function() {
  return require('a2')
};
exports.array = function() { return [1,2,3] };

