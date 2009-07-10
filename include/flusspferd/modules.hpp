// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_IMPORTER_HPP
#define FLUSSPFERD_IMPORTER_HPP

#include "init.hpp"
#include "object.hpp"

namespace flusspferd { 

/**
 * Load the 'require()' function into @p container.
 *
 * Creates a new instance of the require function.
 *
 * @param container The object to load the function into.
 *
 * @ingroup loadable_modules
 */
void load_require_function(object container);

/**
 * The prototype for module loader functions.
 *
 * Modules should define a %function @c flusspferd_load (with
 * <code>extern "C"</code>) with this signature.
 *
 * However, they should <b>not</b> define this %function directly but rather
 * use either #FLUSSPFERD_LOADER or #FLUSSPFERD_LOADER_SIMPLE, which will adapt
 * to changes in the signature.
 *
 * @ingroup loadable_modules
 */
typedef void (*flusspferd_load_t)(object &exports, object &context);

/**
 * Define a module loader.
 *
 * The parameter @p context contains the module's scope, which includes
 * the @c require function. You can access it with
 *
 * @code
 * context.get_property_object("require")
 * @endcode
 *
 * or
 *
 * @code
 * context.call("require", "module-name")
 * @endcode
 *
 * @param exports The object containing the module exports.
 * @param context The root object for the module's scope.
 *
 * @ingroup loadable_modules
 */
#define FLUSSPFERD_LOADER(exports, context) \
  extern "C" \
  void flusspferd_load( \
    ::flusspferd::object &exports, \
    ::flusspferd::object &context)

/**
 * Define a module loader ("simple": without context parameter).
 *
 * @param exports The object containing the module exports.
 *
 * @ingroup loadable_modules
 */
#define FLUSSPFERD_LOADER_SIMPLE(exports) \
  extern "C" \
  void flusspferd_load( \
    ::flusspferd::object &exports, \
    ::flusspferd::object &)
}

#endif
