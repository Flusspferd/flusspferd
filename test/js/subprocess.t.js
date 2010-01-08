const subprocess = require('subprocess');
const asserts = require('test').asserts;

exports.test_cat = function() {
    var args = [ require('flusspferd').executableName, '-e',
                 'var line, sys = require("system"); while( (line = sys.stdin.readLine()) ) { sys.stdout.write(line); sys.stdout.flush(); }',
                 '-c', '/dev/null' ];
    var p = subprocess.popen(args);
    const data = 'hello world\n';
    p.stdin.write(data);
    p.stdin.flush();
    asserts.same(p.stdout.read(data.length), data);
    asserts.ok(p.poll() === null);
    p.terminate();
    p.wait();
    asserts.ok(p.poll() !== null);
};

if (require.main === module)
  require('test').runner(exports);
