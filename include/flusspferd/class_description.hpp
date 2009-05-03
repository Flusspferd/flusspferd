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

#include <boost/preprocessor.hpp>

#define FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS2(x, y) \
  ((x, y)) \
  FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS

#define FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS(x, y) \
  ((x, y)) \
  FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS2

#define FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS_ELIM
#define FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS2_ELIM

#define FLUSSPFERD_PP_GEN_TUPLE2SEQ(x) \
  BOOST_PP_CAT(BOOST_PP_EXPAND(FLUSSPFERD_PP_GEN_TUPLE2SEQ_PROCESS x), _ELIM)

#define FLUSSPFERD_CD_PARAM_FOLD(s, state, elem) \
  BOOST_PP_ARRAY_REPLACE( \
    state, \
    BOOST_PP_EXPAND( \
      BOOST_PP_CAT(FLUSSPFERD_CD_PARAM__, \
        BOOST_PP_TUPLE_ELEM(2, 0, elem))), \
    BOOST_PP_TUPLE_ELEM(2, 1, elem)) \
  /* */

#define FLUSSPFERD_CD_PARAM_INITIAL \
  (2, ( \
    ~name~,      /* name */ \
    BOOST_PP_NIL /* methods */ \
  )) \
  /* */

#define FLUSSPFERD_CD_PARAM__name      0
#define FLUSSPFERD_CD_PARAM__methods   1

#define FLUSSPFERD_CD_PARAM(tuple_seq) \
  BOOST_PP_SEQ_FOLD_LEFT( \
    FLUSSPFERD_CD_PARAM_FOLD, \
    FLUSSPFERD_CD_PARAM_INITIAL, \
    FLUSSPFERD_PP_GEN_TUPLE2SEQ(tuple_seq) \
  ) \
  /* */

#define FLUSSPFERD_CLASS_DESCRIPTION_A(array) \
  FLUSSPFERD_CLASS_DESCRIPTION_P( \
    BOOST_PP_ARRAY_ELEM(0, array), \
    BOOST_PP_ARRAY_ELEM(1, array)) \
  /* */

#define FLUSSPFERD_CLASS_DESCRIPTION_P(name, methods) \
  { name, methods }
  /* */

#define FLUSSPFERD_CLASS_DESCRIPTION(tuple_seq) \
  FLUSSPFERD_CLASS_DESCRIPTION_A(FLUSSPFERD_CD_PARAM(tuple_seq)) \
  /* */

#endif
