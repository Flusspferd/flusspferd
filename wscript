# vim:ts=2:sw=2:expandtab:autoindent:filetype=python:
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
import Utils

VERSION = 'dev'
APPNAME = 'flusspferd'

srcdir = '.'
blddir = 'build'

def init(): pass

def set_options(opt):
    opt.tool_options('compiler_cxx')
    opt.tool_options('boost')

def configure(conf):
    u = conf.env.append_unique
    darwin = sys.platform.startswith('darwin')
    conf.check_message('platform', '', 1, sys.platform)

    print '%s :' % 'Creating implementation link'.ljust(conf.line_just),
    try: os.unlink('include/flusspferd/implementation')
    except OSError: pass
    os.symlink('spidermonkey', 'include/flusspferd/implementation')
    Utils.pprint('GREEN', 'ok')

    if darwin:
        u('CXXDEFINES', 'APPLE')

    u('CXXFLAGS', '-pipe -Wno-long-long -Wall -W -pedantic -std=c++98')

    #u('CXXFLAGS', '-O3 -DNDEBUG')
    u('CXXFLAGS', '-O0 -g -DDEBUG')

    conf.check_tool('compiler_cxx')
    conf.check_tool('misc')
    conf.check_tool('boost')

    conf.env['CXXFLAGS_GCOV'] = '-fprofile-arcs -ftest-coverage'
    conf.env['LINKFLAGS_GCOV'] = '-fprofile-arcs -ftest-coverage'

    boostconf = conf.create_boost_configurator()
    boostconf.lib = ['unit_test_framework', 'thread']
    boostconf.static = 'nostatic'
    boostconf.threadingtag = 'st'
    boostconf.run()
    
    # spidermonkey
    u('CXXDEFINES', 'XP_UNIX') # TODO
    u('CXXDEFINES', 'JS_C_STRINGS_ARE_UTF8')
    libconf = conf.create_library_configurator()
    libconf.name = 'js'
    libconf.path = ['/usr/lib', '/usr/local/lib', '/opt/local/lib', '/sw/lib']
    libconf.mandatory = True
    libconf.run()

    headconf = conf.create_header_configurator()
    headconf.name = 'js/jsapi.h'
    headconf.mandatory = True
    headconf.path = ['/usr/include', '/usr/local/include', '/opt/local/include',
                     '/sw/include']
    headconf.uselib_store = 'JS'
    headconf.run()

def build_pkgconfig(bld):
    obj = bld.new_task_gen('subst')
    obj.source = 'flusspferd.pc.in'
    obj.target = 'flusspferd.pc'
    obj.dict = {
        'PREFIX': bld.env['PREFIX'],
        'LIBDIR': os.path.normpath(bld.env['PREFIX'] + '/lib'),
        'INCLUDEDIR': os.path.normpath(bld.env['PREFIX'] + '/include'),
        'VERSION': VERSION
        }
    obj.install_path = os.path.normpath(bld.env['PREFIX'] + '/lib/pkgconfig/')
    obj.apply()

def build(bld):
    bld.add_subdirs('src test sandbox')
    build_pkgconfig(bld)
    bld.install_files('${PREFIX}/include/templar', 'include/templar/*.hpp')
    bld.install_files('${PREFIX}/lib/pkgconfig', 'flusspferd.pc')

def shutdown(): pass
