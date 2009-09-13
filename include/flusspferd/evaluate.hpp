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

#ifndef FLUSSPFERD_EVALUATE_HPP
#define FLUSSPFERD_EVALUATE_HPP

#include "detail/api.hpp"
#include "object.hpp"

namespace flusspferd {

class value;

/**
 * @name Evaluating Javascript source code
 * @addtogroup evaluate_compile
 */
//@{

/**
 * Evaluate Javascript code.
 *
 * Uses the global object as scope.
 *
 * @param source The source code.
 * @param n The length of the source code in bytes.
 * @param file The file name to use.
 * @param line The initial line number.
 */
value FLUSSPFERD_PUBLIC_API evaluate(char const *source, std::size_t n,
               char const *file = 0x0, unsigned int line = 0);

/**
 * Evaluate Javascript code in a scope.
 *
 * @param source The source code.
 * @param n The length of the source code in bytes.
 * @param file The file name to use.
 * @param line The initial line number.
 * @param scope The scope 
 */
value FLUSSPFERD_PUBLIC_API evaluate_in_scope(char const* source, std::size_t n,
                        char const* file, unsigned int line,
                        object const &scope);

/**
 * Evaluate Javascript code.
 *
 * Uses the global object as context.
 *
 * @param source The source code.
 * @param file The file name to use.
 * @param line The initial line number.
 */
value FLUSSPFERD_PUBLIC_API evaluate(char const *source, char const *file = 0x0,
               unsigned int line = 0);

/**
 * Evaluate Javascript code.
 *
 * Uses the global object as context.
 *
 * @param source The source code.
 * @param file The file name to use.
 * @param line The initial line number.
 */
value FLUSSPFERD_PUBLIC_API evaluate(std::string const &source, char const *file = 0x0,
               unsigned int line = 0);

/**
 * Execute a Javascript file.
 *
 * @param file The path to the file.
 * @param scope The scope to use.
 */
value FLUSSPFERD_PUBLIC_API execute(char const *file, object const &scope = object());

//@}

}

#endif /* FLUSSPFERD_EVALUATE_HPP */
