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

#ifndef FLUSSPFERD_EXCEPTION_HPP
#define FLUSSPFERD_EXCEPTION_HPP

#include <boost/exception/exception.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

namespace flusspferd {

class value;

/**
 * Exception class.
 *
 * @ingroup exceptions
 */
struct exception : virtual std::runtime_error, virtual boost::exception {
  /**
   * Constructor.
   *
   * If available, this constructor will create an exception containing the last
   * Javascript engine error. Otherwise, it will create an exception of type
   * @p type.
   *
   * @param what The error message.
   * @param type The error type (if applicable).
   */
  exception(char const *what, std::string const &type = "Error")
  : std::runtime_error(exception_message(what))
  {
    init(what, type);
  }

  /**
   * Constructor.
   *
   * If available, this constructor will create an exception containing the last
   * Javascript engine error. Otherwise, it will create an exception of type
   * @p type.
   *
   * @param what The error message.
   * @param type The error type (if applicable).
   */
  exception(std::string const &what, std::string const &type = "Error")
  : std::runtime_error(exception_message(what.c_str()))
  {
    init(what.c_str(), type);
  }

  /**
   * Value constructor.
   *
   * Will create an exception containing @p val. The error message will be the
   * string representation of @p val.
   */
  exception(value const &val);

  /// Destructor.
  ~exception() throw();

  /**
   * The exception value.
   *
   * @return The exception value.
   */
  value val() const;

  /**
   * Will return true only if the exception contains an exception fetched from the
   * underlying Javascript engine.
   *
   * @return Whether this exception is a JavaScript exception.
   */
  bool is_js_exception() const;

public:
#ifndef IN_DOXYGEN
  void throw_js_INTERNAL();
#endif

private:
  void init(char const *what, std::string const &type);
  static std::string exception_message(char const *what);

private:
  class impl;
  boost::shared_ptr<impl> p;
};

/**
 * Indicator exception.
 *
 * If thrown inside a script or a Javascript function, this will quit
 * the script immediately.
 *
 * @ingroup exceptions
 */
class js_quit {
public:
  /// Constructor.
  js_quit();

  /// Destructor.
  virtual ~js_quit();
};

}

#ifndef IN_DOXYGEN

#define FLUSSPFERD_CALLBACK_BEGIN try

#define FLUSSPFERD_CALLBACK_END \
    catch (::flusspferd::exception &e) { \
      e.throw_js_INTERNAL(); \
      return JS_FALSE; \
    } catch (::std::exception &e) { \
      ::flusspferd::exception x(e.what()); \
      x.throw_js_INTERNAL(); \
      return JS_FALSE; \
    } catch (::flusspferd::js_quit&) {\
      return JS_FALSE; \
    } \
    return JS_TRUE

#endif

#endif /* FLUSSPFERD_EXCEPTION_HPP */
