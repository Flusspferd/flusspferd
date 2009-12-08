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

#ifndef FLUSSPFERD_IMPORTER_HPP
#define FLUSSPFERD_IMPORTER_HPP

#include "init.hpp"
#include "object.hpp"
#include "array.hpp"
#include "native_function_base.hpp"
#include <boost/filesystem.hpp>

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

/// Functor object that backs the require('foo') in JavaScript
class require : public flusspferd::native_function_base {
public:
  typedef boost::mpl::true_ ignore_name_arity;

  require(function const &fun);
  require(function const &fun, require const &rhs);
  ~require();

  /// The different types of IDs understood by require
  enum id_classification {
    relative, // "./foo" or "../foo"
    top_level, // "foo/bar"
    fully_qualified // "file://path/to/foo.js"
  };

  void call(flusspferd::call_context &x);

  /// Create the initial %require function.
  /**
   * Needed because native_function_base doesn't have a nice way to create
   * properties on the function object itself
   */
  static object create_require();

  static string load_module_text(boost::filesystem::path filename);

  /// Create a sub-%require object for the given module id
  object new_require_function(string const &id);

  /// Populate values in require.main for @c id
  void set_main_module(std::string const &id);

protected:
  object module_cache;
  object paths;
  object alias;
  object preload;
  object main;


  std::string current_id();

  object load_top_level_module(std::string &id);
  object load_absolute_js_file(boost::filesystem::path path, std::string &id);

  boost::filesystem::path resolve_relative_id(std::string const &id);

  void require_js(boost::filesystem::path filename,
                  std::string const &id,
                  object exports);

  static id_classification classify_id(std::string const &id);

  boost::optional<boost::filesystem::path>
  find_top_level_js_module(std::string const & id, bool not_found_is_fatal);

};

} // namespace flusspferd

#endif
