// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#ifndef FLUSSFPERD_CLASS_DESCRIPTION_HPP
#define FLUSSFPERD_CLASS_DESCRIPTION_HPP

#ifndef PREPROC_DEBUG
#include "class.hpp"
#include "native_object_base.hpp"
#endif
#include <boost/preprocessor.hpp>

/* 2-tuple seq */

#define FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS2(x, y) \
  ((x, y)) \
  FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS

#define FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS(x, y) \
  ((x, y)) \
  FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS2

#define FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS_ELIM
#define FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS2_ELIM

#define FLUSSPFERD_PP_GEN_TUPLE2SEQ(x) \
  BOOST_PP_CAT(FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS x, _ELIM)

/* 3-tuple seq */

#define FLUSSPFERD_PP_GEN_TUPLE3SEQ_PROCESS2(x, y, z) \
  ((x, y, z)) \
  FLUSSPFERD_PP_GEN_TUPLE3SEQ_PROCESS

#define FLUSSPFERD_PP_GEN_TUPLE3SEQ_PROCESS(x, y, z) \
  ((x, y, z)) \
  FLUSSPFERD_PP_GEN_TUPLE3SEQ_PROCESS2

#define FLUSSPFERD_PP_GEN_TUPLE3SEQ_PROCESS_ELIM
#define FLUSSPFERD_PP_GEN_TUPLE3SEQ_PROCESS2_ELIM

#define FLUSSPFERD_PP_GEN_TUPLE3SEQ(x) \
  BOOST_PP_CAT(FLUSSPFERD_PP_GEN_TUPLE3SEQ_PROCESS x, _ELIM)

/* -- */

#define FLUSSPFERD_CD_PARAM_FOLD(s, state, elem) \
  BOOST_PP_ARRAY_REPLACE( \
    state, \
    BOOST_PP_EXPAND( \
      BOOST_PP_CAT(FLUSSPFERD_CD_PARAM__, \
        BOOST_PP_TUPLE_ELEM(2, 0, elem))), \
    BOOST_PP_TUPLE_ELEM(2, 1, elem)) \
  /* */

#define FLUSSPFERD_CD_PARAM_INITIAL \
  (10, ( \
    ~cpp_name~,                        /* name */ \
    ~constructor_name~,                /* constructor name */ \
    0,                                 /* constructor arity */ \
    ~full_name~,                       /* full name */ \
    ~methods~,                         /* methods */ \
    0,                                 /* NO methods */ \
    0,                                 /* augment constructor */ \
    true,                              /* constructible */ \
    false,                             /* custom enumerate */ \
    ::flusspferd::native_object_base   /* base class */ \
  )) \
  /* */

#define FLUSSPFERD_CD_PARAM__cpp_name               0
#define FLUSSPFERD_CD_PARAM__constructor_name       1
#define FLUSSPFERD_CD_PARAM__constructor_arity      2
#define FLUSSPFERD_CD_PARAM__full_name              3
#define FLUSSPFERD_CD_PARAM__methods                4
#define FLUSSPFERD_CD_PARAM__no_methods             5
#define FLUSSPFERD_CD_PARAM__augment_constructor    6
#define FLUSSPFERD_CD_PARAM__constructible          7
#define FLUSSPFERD_CD_PARAM__custom_enumerate       8
#define FLUSSPFERD_CD_PARAM__base                   9

#define FLUSSPFERD_CD_PARAM(tuple_seq) \
  BOOST_PP_SEQ_FOLD_LEFT( \
    FLUSSPFERD_CD_PARAM_FOLD, \
    FLUSSPFERD_CD_PARAM_INITIAL, \
    FLUSSPFERD_PP_GEN_TUPLE2SEQ(tuple_seq) \
  ) \
  /* */

#define FLUSSPFERD_CLASS_DESCRIPTION_A(array) \
  BOOST_PP_EXPAND(FLUSSPFERD_CLASS_DESCRIPTION_P BOOST_PP_ARRAY_DATA(array)) \
  /* */

#define FLUSSPFERD_CLASS_DESCRIPTION_P( \
  p_cpp_name, \
  p_constructor_name, \
  p_constructor_arity, \
  p_full_name, \
  p_methods, \
  p_no_methods, \
  p_augment_constructor, \
  p_constructible, \
  p_custom_enumerate, \
  p_base \
) \
  class p_cpp_name : public p_base { \
  public: \
    struct class_info : ::flusspferd::class_info { \
      typedef boost::mpl::bool_< (p_constructible) > constructible; \
      static char const *constructor_name() { \
        return (p_constructor_name); \
      } \
      typedef ::boost::mpl::size_t< (p_constructor_arity) > constructor_arity; \
      static char const *full_name() { \
        return (p_full_name); \
      } \
      static ::flusspferd::object create_prototype() { \
        ::flusspferd::object proto = ::flusspferd::create_object( \
            ::flusspferd::prototype< p_base >() \
          ); \
        BOOST_PP_IF( \
          p_no_methods, \
          BOOST_PP_TUPLE_EAT(2), \
          FLUSSPFERD_CD_METHODS \
        ) (p_cpp_name, p_methods) \
        return proto; \
      } \
      static void augment_constructor(::flusspferd::object &o) { \
        (void)o; \
        BOOST_PP_EXPR_IF( \
          p_augment_constructor, \
          p_cpp_name :: augment_constructor(o);) \
      } \
      typedef boost::mpl::bool_< (p_custom_enumerate) > custom_enumerate; \
    }; \
    BOOST_PP_EXPR_IF( \
      p_augment_constructor, \
      static void augment_constructor(::flusspferd::object &o);) \
  private: \
  /* */

#define FLUSSPFERD_CD_METHODS(p_cpp_name, p_methods) \
  BOOST_PP_SEQ_FOR_EACH( \
    FLUSSPFERD_CD_METHOD, \
    p_cpp_name, \
    FLUSSPFERD_PP_GEN_TUPLE3SEQ(p_methods)) \
  /* */

#define FLUSSPFERD_CD_METHOD(r, p_cpp_name, p_method) \
  BOOST_PP_CAT( \
    FLUSSPFERD_CD_METHOD__, \
    BOOST_PP_TUPLE_ELEM(3, 1, p_method) \
  ) ( \
    p_cpp_name, \
    BOOST_PP_TUPLE_ELEM(3, 0, p_method), \
    BOOST_PP_TUPLE_ELEM(3, 2, p_method) \
  ) \
  /* */

#define FLUSSPFERD_CD_METHOD__bind(p_cpp_name, p_method_name, p_bound) \
  ::flusspferd::create_native_method( \
      proto, \
      (p_method_name), \
      & p_cpp_name :: p_bound); \
  /* */

#define FLUSSPFERD_CD_METHOD__alias(p_cpp_name, p_method_name, p_alias) \
  proto.define_property( \
    (p_method_name), \
    proto.get_property((p_alias)), \
    ::flusspferd::dont_enumerate); \
  /* */

#define FLUSSPFERD_CLASS_DESCRIPTION(tuple_seq) \
  FLUSSPFERD_CLASS_DESCRIPTION_A(FLUSSPFERD_CD_PARAM(tuple_seq)) \
  /* */

#define FLUSSPFERD_CLASS_DESCRIPTION_END() \
  }; \
  /* */

#endif
