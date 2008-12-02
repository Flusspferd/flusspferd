// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:

#ifndef FLUSSPFERD_IMPORTER_HPP
#define FLUSSPFERD_IMPORTER_HPP

#include "native_object_base.hpp"
#include "class.hpp"
#include <boost/scoped_ptr.hpp>

namespace flusspferd { 

class importer : public native_object_base {

public:
  struct class_info : flusspferd::class_info {
    typedef boost::mpl::bool_<true> constructible;
    static char const* constructor_name() { return "Importer"; }
    static object create_prototype();
    static void augment_constructor(object &);
  };

  importer(object const &obj, call_context &x);
  ~importer();

protected:
  void trace(tracer &);

  static std::string process_name(string const &name, bool for_script = false);

private: // JS methods
  value load(string const &name, bool binary_only); 

private:
  class impl;
  boost::scoped_ptr<impl> p;
};

}

#endif
