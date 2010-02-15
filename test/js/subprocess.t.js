try {

const subprocess = require('subprocess'),
      asserts = require('test').asserts,
      WIN32 = "APPDATA" in require('system').env,
      dev_null = WIN32 ? "NUL" : "/dev/null";

// Helper fn to deal with different line ending on various platforms
function lines() {
  return Array.join(arguments, WIN32 ? "\r\n" : "\n") + (WIN32 ? "\r\n" : "\n");
}

exports.test_shell = function() {
    const data = "hello world";
    const cmd = require('flusspferd').executableName +
        ' -e \'const out = require("system").stdout; out.write("' + data + '"); out.flush()\' -c ' + dev_null;

  // Had some issues where spawning multiple process and calling communicate
  //would fail the second time round.
  for (var i =0; i < 3; i++) {
    var p = subprocess.popen(cmd);

    //print("poll:", p.poll() );
    var r = p.communicate();
    asserts.same(r.returncode, 0, "exit code 0");
    asserts.same(r.stdout, data, "stdout ok");
  }
};

exports.test_cat = function() {
    var args = [ require('flusspferd').executableName, '-e',
                 'var line, sys = require("system"); while( (line = sys.stdin.readLine()) ) { sys.stdout.write(line); sys.stdout.flush(); }',
                 '-c', dev_null ];
    var p = subprocess.popen(args);
    asserts.ok(p.stdin, "have stdin stream");
    asserts.ok(p.stdout, "have stdout stream");
    asserts.ok(p.stderr, "have stderr stream");

    asserts.ok(p.poll() === null, "poll returns null when process still alive");

    const data = lines('hello world', 'line 2');
    p.stdin.write(data);
    p.stdin.flush();
    asserts.same(p.stdout.read(data.length), data);

    p.terminate();

    var ret = p.wait();
    asserts.ok(ret !== null, "return code is non-null");

    asserts.same(p.returncode, ret);
    asserts.same(p.poll(), ret);
};

exports.test_communicate = function() {
    var args = [ require('flusspferd').executableName, '-e',
                 'const out = require("system").stdout; out.write("hello world\\n"); out.flush();',
                 '-c', dev_null
               ];

    var p = subprocess.popen(args, "r");
    asserts.same(p.stdin, undefined, "stdin is closed");
    asserts.same(p.stderr, undefined, "stderr is closed");
    asserts.ok(p.stdout !== undefined, "stdout is opened");

    var r = p.communicate();

    asserts.same(r.returncode, 0, "returncode is 0");
    asserts.same(p.poll(), r.returncode, "returncode returned from communicate");
    asserts.same(r.returncode, p.returncode, "returncode stored on object");
    asserts.same(r.stdout, lines("hello world"), "stdout correct");
    asserts.same(r.stderr, null, "stderr correct");
};

exports.test_retcode = function() {
    const retval = 12;
    const args = [ require('flusspferd').executableName, '-e',
                   'quit(' + retval + ');',
                   '-c', dev_null
                 ];
    var p = subprocess.popen({ args : args, stdin : false, stderr : false, stdout : false });

    asserts.same(p.stdin, undefined, "stdin is closed");
    asserts.same(p.stderr, undefined, "stderr is closed");
    asserts.same(p.stdout, undefined, "stdout is closed");

    asserts.same(p.wait(), retval, "wait() correct");
    asserts.same(p.poll(), retval, "poll() correct");
    asserts.same(p.returncode, retval, "returncode correct");
};

}
catch(e if e.message && e.message.match(/'subprocess'/)) {
  // this sucks we really should change the exception system (#44)
  exports.test_skip = function() {
    require('test').asserts.diag("Not running subprocess test (Module not built)");
  };
}

if (require.main === module)
  require('test').runner(exports);
