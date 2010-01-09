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
    var ret = p.wait();
    asserts.same(p.returncode, ret);
    asserts.same(p.poll(), ret);
};

exports.test_communicate = function() {
    var args = [ require('flusspferd').executableName, '-e',
                 'const out = require("system").stdout; out.write("hello world\\n"); out.flush();',
                 '-c', '/dev/null'
               ];
    var p = subprocess.popen(args, "r");
    var r = p.communicate();
    asserts.same(p.poll(), r.returncode);
    asserts.same(r.returncode, p.returncode);
    asserts.same(r.stdout, "hello world\n");
    asserts.same(r.stderr, "");
};

exports.test_retcode = function() {
    const retval = 12;
    const args = [ require('flusspferd').executableName, '-e',
                   'quit(' + retval + ');',
                   '-c', '/dev/null'
                 ];
    var p = subprocess.popen({ args : args, stdin : false, stderr : false, stdout : false });
    asserts.same(p.wait(), retval);
    asserts.same(p.poll(), retval);
    asserts.same(p.returncode, retval);
};

if (require.main === module)
  require('test').runner(exports);
