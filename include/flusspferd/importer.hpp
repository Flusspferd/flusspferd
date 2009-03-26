// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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
 * Load the 'Import()' function into @p container.
 *
 * Creates a new instance of the Import function.
 *
 * @param container The object to load the function into.
 *
 * @ingroup loadable_modules
 * @ingroup jsext
 */
void load_import_function(object container = flusspferd::global());

/**
 * The prototype for module loader functions.
 *
 * Modules should define a function @c flusspferd_load (with 
 * <code>extern "C"</code>) with this signature.
 *
 * Example:
 * @dontinclude help/examples/dummy_module.cpp
 * @skip extern "C"
 * @until }
 *
 * @ingroup loadable_modules
 */
typedef value (*flusspferd_load_t)(object container);

}

#endif
