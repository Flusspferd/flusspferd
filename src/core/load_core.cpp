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

#include "flusspferd/load_core.hpp"
#include "flusspferd/modules.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/properties_functions.hpp"
#include "flusspferd/binary.hpp"
#include "flusspferd/encodings.hpp"
#include "flusspferd/system.hpp"
#include "flusspferd/getopt.hpp"
#include "flusspferd/io/io.hpp"
#include "flusspferd/io/filesystem-base.hpp"
#include "flusspferd/create.hpp"

#include <boost/filesystem.hpp>
#include <boost/spirit/home/phoenix/core.hpp>
#include <boost/spirit/home/phoenix/bind.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
namespace phoenix = boost::phoenix;
namespace args = phoenix::arg_names;

using namespace flusspferd;

namespace flusspferd { namespace detail {
  extern char const * const json2;
}}

void flusspferd::load_core(object const &scope_, std::string const &argv0) {
  root_object scope(scope_);

  // Initalize boost's copy of cwd as early as possible
  boost::filesystem::initial_path<boost::filesystem::path>();

  flusspferd::load_require_function(scope);
  flusspferd::load_properties_functions(scope);

  flusspferd::object require_fn = scope.get_property_object("require");

  // Create the top level |module| and |exports| properties.
  scope.define_property("module", require_fn.get_property("main"), dont_enumerate);

  root_object exports(create_object());
  scope.define_property("exports", exports, dont_enumerate);

  root_object preload(require_fn.get_property_object("preload"));

  flusspferd::create_native_method(
    preload, "binary",
    &flusspferd::load_binary_module);

  flusspferd::create_native_method(
    preload, "encodings",
    &flusspferd::load_encodings_module);

  flusspferd::create_native_method(
    preload, "io",
    &flusspferd::io::load_io_module);

  flusspferd::create_native_method(
    preload, "system",
    &flusspferd::load_system_module);

  flusspferd::create_native_method(
    preload, "getopt",
    &flusspferd::load_getopt_module);

  flusspferd::create_native_method(
    preload, "filesystem-base",
    &flusspferd::load_filesystem_base_module);
  // alias fs-base -> filesystem-base
  require_fn.get_property_object("alias").set_property("fs-base", "filesystem-base");

  // Curry argv[0] into the preload function
  flusspferd::create_native_method(
    preload, "flusspferd",
    boost::function<void (object)>((
      phoenix::bind(&flusspferd::load_flusspferd_module, args::arg1, argv0)
    )));

  if (!scope_.has_own_property("JSON")) {
    flusspferd::evaluate_in_scope(
      detail::json2,
      std::strlen(detail::json2),
      0x0, // Filename
      0,   // Linenumber
      scope_);
  }
}
