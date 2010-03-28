// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <flusspferd/create_on.hpp>
#include <flusspferd/evaluate.hpp>

#include <iostream>
#include "test_environment.hpp"

using namespace flusspferd;
using namespace std;

void callback(flusspferd::object fn){
  fn.call(global());
}

BOOST_AUTO_TEST_CASE(bug_159) {  
  try {
    init::initialize();
    context co = context::create();
    current_context_scope scope(co);
    
    object g = global();
    security::create(g);
    load_core(g, "bug159");
    
    create_on(g)
      .create<flusspferd::function>("gc", &gc)
      .create<flusspferd::function>("execute", &callback);

    /* FIXME path finding */
    #define P "../../test/fixtures/bug159"

    g.get_property_object("require")
      .get_property_object("paths")
      .call("unshift", P);

    gc();
    
    execute(P "/test.js");   
  } catch (flusspferd::exception &fe) {
    std::cerr << fe.what();
    BOOST_CHECK(!"Flusspferd exception");
  } catch (std::exception &e) {
    std::cerr << e.what();
    BOOST_CHECK(!"Std exception");
  }
}
