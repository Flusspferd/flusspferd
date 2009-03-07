# vim:ts=4:sw=4:expandtab:autoindent:filetype=python:
#
# Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

import sys, os
import Utils, Options

VERSION = 'dev'
APPNAME = 'flusspferd'

srcdir = '.'
blddir = 'build'
darwin = sys.platform.startswith('darwin')

def init(): pass

def set_options(opt):
    opt.tool_options('compiler_cxx')
    opt.tool_options('compiler_cc')
    opt.tool_options('boost')
    opt.add_option('--with-cxxflags', action='store', nargs=1, dest='cxxflags',
                   help='Set non-standard CXXFLAGS')
    opt.add_option('--enable-tests', action='store_true',
                   help='Enable tests')
    opt.add_option('--enable-sandbox', action='store_true',
                   help='Enable sandbox tests')
    opt.add_option('--enable-io', action='store_true',
                   help='Enable IO support')
    opt.add_option('--enable-xml', action='store_true',
                   help='Enable XML support')
    opt.add_option('--enable-curl', action='store_true',
                   help='Build cURL extension')
    opt.add_option('--enable-sqlite', action='store_true', help='Enable SQLite plugin')
    opt.add_option('--with-spidermonkey-include', action='store', nargs=1,
                   dest='spidermonkey_include',
                   help='spidermonkey include path without the js/')
    opt.add_option('--with-spidermonkey-library', action='store', nargs=1,
                   dest='spidermonkey_library',
                   help='spidermonkey library path')
    opt.add_option('--with-spidermonkey', action='store', nargs=1,
                   dest='spidermonkey_path',
                   help='''
base path for spidermonkey. /include and /lib are added. (overwrites
--with-spidermonkey-include/-library)
''')
    if darwin:
      opt.add_option('--libxml-framework', action='store', nargs=1,
                     dest='libxml_framework',
                     help='libxml framework name')

def append_each_unique(env, to, what, split=' '):
    for i in what.split(split):
        env.append_unique(to, i)

def configure(conf):
    u = conf.env.append_unique
    conf.check_message('platform', '', 1, sys.platform)

    print '%s : ' % 'Creating implementation link'.ljust(conf.line_just),
    sys.stdout.flush()
    try: os.unlink('include/flusspferd/implementation')
    except OSError: pass
    os.symlink('spidermonkey', 'include/flusspferd/implementation')
    Utils.pprint('GREEN', 'ok')

    if darwin:
        u('CXXDEFINES', 'APPLE')
        # Is there a better way of doing this?
        if 'FRAMEWORKPATH' in os.environ:
            u('FRAMEWORKPATH', os.environ['FRAMEWORKPATH'] )

    if Options.options.cxxflags:
        conf.env['CXXFLAGS'] = str(Options.options.cxxflags)
    else:
        append_each_unique(conf.env, 'CXXFLAGS',
                           '-pipe -Wno-long-long -Wall -W -pedantic -std=c++98')
        #append_each_unique(conf.env, 'CXXFLAGS', '-O0 -g -DNDEBUG')
        append_each_unique(conf.env, 'CXXFLAGS', '-O0 -g -DDEBUG')

    conf.check_tool('compiler_cxx')
    conf.check_tool('compiler_cc')
    conf.check_tool('misc')
    conf.check_tool('boost')
    if darwin:
        conf.check_tool('osx')

    conf.env['CXXFLAGS_GCOV'] = '-fprofile-arcs -ftest-coverage'
    conf.env['LINKFLAGS_GCOV'] = '-fprofile-arcs -ftest-coverage'

    boostlib = ['thread', 'filesystem', 'system']
    if Options.options.enable_tests:
      boostlib += ['unit_test_framework']

    # boost
    conf.check_boost(lib = boostlib, min_version='1.36.0', mandatory=1)

    # spidermonkey
    lib_path = []
    include_path = []
    if Options.options.spidermonkey_include:
        include_path = [Options.options.spidermonkey_include]
    if Options.options.spidermonkey_library:
        lib_path = [Options.options.spidermonkey_library]
    if Options.options.spidermonkey_path:
        include_path = [os.path.join(Options.options.spidermonkey_path,
                                    'include')]
        lib_path = [os.path.join(Options.options.spidermonkey_path, 'lib')]

    ret = conf.check_cxx(lib = 'js', uselib_store='JS', libpath=lib_path)
    if ret == False:
        conf.env['LIB_JS'] = []
        conf.check_cxx(lib = 'mozjs', uselib_store='JS', mandatory=1,
                       libpath=lib_path)
    js_h_defines = []
    if not conf.check_cxx(header_name = 'js/js-config.h', includes=include_path,
                          defines=['XP_UNIX', 'JS_C_STRINGS_ARE_UTF8']):
        ret = conf.check_cxx(uselib='JS',
                          includes=include_path, execute=1,
                          defines=['XP_UNIX', 'JS_C_STRINGS_ARE_UTF8',
                                   'JS_THREADSAFE'],
                          msg='Checking if SM needs JS_THREADSAFE',
                          fragment='''
                                   #include <js/jsapi.h>
                                   #include <stdio.h>
                                   int main() {
                                     printf("%p", (void*) JS_BeginRequest);
                                   }
                                ''')
        if ret:
            js_h_defines += ['JS_THREADSAFE']

    conf.check_cxx(header_name = 'js/jsapi.h', mandatory = 1,
                   uselib_store='JS_H',
                   defines=js_h_defines + ['XP_UNIX', 'JS_C_STRINGS_ARE_UTF8'],
                   includes=include_path)
    conf.check_cxx(uselib=['JS_H', 'JS'],
                   mandatory = 1, execute=1,
                   msg='Checking if SM was compiled with UTF8',
                   errmsg='Spidermonkey not compiled with UTF8 Support!',
                   fragment='''
#include <js/jsapi.h>
int main() {
  return JS_CStringsAreUTF8() ? 0 : 1;
}
''')

    # dl
    ret = conf.check_cxx(lib = 'dl', uselib_store='DL')

    # libedit
    if conf.check_cc(lib='edit', uselib_store='EDITLINE') and conf.check_cc(header_name='editline/readline.h'):
        u('CXXDEFINES', 'HAVE_EDITLINE')
        conf.check_cc(header_name='editline/history.h')

    # sqlite
    if Options.options.enable_sqlite:
        conf.check_cxx(header_name = 'sqlite3.h', mandatory = 1,
                       uselib_store='SQLITE', execute=1,
                       errmsg='SQLite 3 (>= 3.4.0) could not be found or the found version is too old.',
                       fragment='''
#include <sqlite3.h>
#include <stdio.h>
int main() {
   if(SQLITE_VERSION_NUMBER <= 3004000) {
     fprintf(stderr, "Need sqlite3 version 3.4.0 or better. Found %s\\n",
             SQLITE_VERSION);
     return 1;
   }
   return 0;
}
''')
        conf.check_cxx(lib = 'sqlite3', mandatory = 1, uselib_store='SQLITE')

    # xml
    if Options.options.enable_xml:
        ret = None
        if darwin and Options.options.libxml_framework:
            u('FRAMEWORK', '-framework ' + Options.options.libxml_framework )
            # TODO: check the version of this framework via XMLVERSION define
            ret = conf.check_cxx(uselib_store='LIBXML2',
                                 framework_name=Options.options.libxml_framework, 
                                 execute=1,
                                 errmsg='framework "libxml-2.0 (>= 2.6.0)" could not be found or the found version is too old.',
                                 fragment='''
#include <libxml/xmlversion.h>
#include <stdio.h>
int main() { 
  LIBXML_TEST_VERSION;
  if (LIBXML_VERSION < 20600) {
    // This error message doesn't actually get used
    printf("Need libxml-2.0 version 2.6.0 minimum, we have %s\\n", LIBXML_DOTTED_VERSION);
    return 1;
  } else {
    return 0;
  }
} ''')
            # This faffing is less than ideal really.
            conf.env['FRAMEWORK'] = []
            conf.env['FRAMEWORK_LIBXML2'] = \
                ['-framework ' + Options.options.libxml_framework ]

        # Non darwin, or framework failed
        if ret == None:
          ret = conf.check_cfg(package = 'libxml-2.0', uselib_store='LIBXML2',
                               atleast_version='2.6.0', args = '--cflags --libs')
        if ret == None:
          conf.check_message_2('No suitable libxml-2.0 found, disabling', color='PINK')
          Options.options.enable_xml = None
        else:
          u('CXXDEFINES', 'FLUSSPFERD_HAVE_XML')

    conf.check_cxx(function_name='fork', 
                   header_name='unistd.h',
                   uselib_store='POSIX',
                   defines=['HAVE_FORK','HAVE_UNISTD_H'])
    conf.check_cxx(function_name='usleep', 
                   header_name='unistd.h',
                   uselib_store='POSIX',
                   defines=['HAVE_USLEEP'])

    if Options.options.enable_io:
        u('CXXDEFINES', 'FLUSSPFERD_HAVE_IO')

    if Options.options.enable_curl:
      if (conf.check_cxx(lib = 'curl', 
                        uselib_store='CURL') != None and 
         conf.check_cxx(header_name = 'curl/curl.h',
                        uselib_store='CURL') != None):
        conf.env['ENABLE_CURL'] = True

    conf.env['ENABLE_TESTS'] = Options.options.enable_tests
    conf.env['ENABLE_SANDBOX'] = Options.options.enable_sandbox
    conf.env['ENABLE_XML'] = Options.options.enable_xml
    conf.env['ENABLE_IO'] = Options.options.enable_io

def get_defines(bld, envvars):
    result = ''
    def_pattern = bld.env['CXXDEFINES_ST']
    for envvar in envvars:
        defines = bld.env['CXXDEFINES_' + envvar]
        if defines:
            for i in defines:
                result += def_pattern % i + ' '
    return result

def build_pkgconfig(bld):
    obj = bld.new_task_gen('subst')
    obj.source = 'flusspferd.pc.in'
    obj.target = 'flusspferd.pc'
    obj.dict = {
        'PREFIX': bld.env['PREFIX'],
        'LIBDIR': os.path.normpath(bld.env['PREFIX'] + '/lib'),
        'INCLUDEDIR': os.path.normpath(bld.env['PREFIX'] + '/include'),
        'VERSION': VERSION,
        'CFLAGS': get_defines(bld, ['JS_H'])
        }
    obj.install_path = os.path.normpath(bld.env['PREFIX'] + '/lib/pkgconfig/')
    obj.apply()

def build(bld):
    bld.add_subdirs('src')
    bld.add_subdirs('programs')
    bld.add_subdirs('plugins/sqlite3')
    bld.add_subdirs('plugins/environment')
    bld.add_subdirs('plugins/posix')
    if bld.env['ENABLE_CURL']:
        bld.add_subdirs('plugins/curl')

    if bld.env['ENABLE_TESTS']:
      bld.add_subdirs('test')
    if bld.env['ENABLE_SANDBOX']:
      bld.add_subdirs('sandbox')
    build_pkgconfig(bld)
    bld.install_files('${PREFIX}/include/flusspferd/',
                      'include/flusspferd/*.hpp')
    bld.install_files('${PREFIX}/include/flusspferd/implementation/',
                      'include/flusspferd/implementation/*.hpp')
    bld.install_files('${PREFIX}/lib/pkgconfig/', 'flusspferd.pc')

    bld.install_files('${PREFIX}/lib/flusspferd/modules', 'js/src/*.js')
    bld.install_files('${PREFIX}/lib/flusspferd/modules/HTTP',
                      'js/src/HTTP/*.js')

    bld.install_files('${PREFIX}/lib/flusspferd/', 'prelude.js')

    bld.symlink_as('${PREFIX}/lib/flusspferd/modules/' +
                   (bld.env['shlib_PATTERN'] % 'XML'),
                   '../../' + (bld.env['shlib_PATTERN'] % 'flusspferd-xml'))
    bld.symlink_as('${PREFIX}/lib/flusspferd/modules/' +
                   (bld.env['shlib_PATTERN'] % 'IO'),
                   '../../' + (bld.env['shlib_PATTERN'] % 'flusspferd-io'))

    etc = bld.new_task_gen('subst')
    etc.source = 'jsrepl.js.in'
    etc.target = 'jsrepl.js'
    etc.dict = {
      'PREFIX': bld.env['PREFIX']
    }
    etc.install_path = os.path.normpath(bld.env['PREFIX'] + '/etc/flusspferd')
    etc.apply();

def shutdown(): pass
