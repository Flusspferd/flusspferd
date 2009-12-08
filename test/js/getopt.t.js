const getopt = require('getopt').getopt;
const asserts = require('test').asserts;

exports.test_aliases = function() {
  var spec = { verbose : { aliases : ["v", "noisey"] } };
  asserts.same(
    getopt( spec, [ "-v", "--noisey" ] ),
    { "verbose": [undefined, undefined], _: [] },
    "Aliases keyed using full name" 
  );
}

if (require.main === module)
  require('test').runner(exports);
