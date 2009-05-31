// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
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

#ifndef FLUSSFPERD_CLASS_DESCRIPTION_HPP
#define FLUSSFPERD_CLASS_DESCRIPTION_HPP

#ifndef PREPROC_DEBUG
#include "class.hpp"
#include "native_object_base.hpp"
#endif
#include "detail/limit.hpp"
#include <boost/preprocessor.hpp>

#ifndef IN_DOXYGEN

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
  (13, ( \
    ~cpp_name~,                        /* name */ \
    ::flusspferd::native_object_base,  /* base class */ \
    ~constructor_name~,                /* constructor name */ \
    0,                                 /* constructor arity */ \
    true,                              /* constructible */ \
    ~full_name~,                       /* full name */ \
    (~, none, ~),                      /* methods */ \
    (~, none, ~),                      /* constructor methods */ \
    (~, none, ~),                      /* properties */ \
    (~, none, ~),                      /* constructor properties */ \
    false,                             /* custom enumerate */ \
    0,                                 /* augment constructor (custom func.)*/\
    0                                  /* augment prototype (custom func.) */ \
  )) \
  /* */

#define FLUSSPFERD_CD_PARAM__cpp_name                 0
#define FLUSSPFERD_CD_PARAM__base                     1
#define FLUSSPFERD_CD_PARAM__constructor_name         2
#define FLUSSPFERD_CD_PARAM__constructor_arity        3
#define FLUSSPFERD_CD_PARAM__constructible            4
#define FLUSSPFERD_CD_PARAM__full_name                5
#define FLUSSPFERD_CD_PARAM__methods                  6
#define FLUSSPFERD_CD_PARAM__constructor_methods      7
#define FLUSSPFERD_CD_PARAM__properties               8
#define FLUSSPFERD_CD_PARAM__constructor_properties   9
#define FLUSSPFERD_CD_PARAM__custom_enumerate        10
#define FLUSSPFERD_CD_PARAM__augment_constructor     11
#define FLUSSPFERD_CD_PARAM__augment_prototype       12

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
  p_base, \
  p_constructor_name, \
  p_constructor_arity, \
  p_constructible, \
  p_full_name, \
  p_methods, \
  p_constructor_methods, \
  p_properties, \
  p_constructor_properties, \
  p_custom_enumerate, \
  p_augment_constructor, \
  p_augment_prototype \
) \
  template<typename Class> \
  class BOOST_PP_CAT(p_cpp_name, _base) : public p_base { \
  public: \
    typedef BOOST_PP_CAT(p_cpp_name, _base) base_type; \
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
        ::flusspferd::object obj = ::flusspferd::create_object( \
            ::flusspferd::prototype< p_base >() \
          ); \
        FLUSSPFERD_CD_METHODS(p_methods) \
        FLUSSPFERD_CD_PROPERTIES(p_properties) \
        BOOST_PP_EXPR_IF( \
          p_augment_prototype, \
          Class :: augment_prototype(obj);) \
        return obj; \
      } \
      static void augment_constructor(::flusspferd::object &obj) { \
        (void)obj; \
        FLUSSPFERD_CD_METHODS(p_constructor_methods) \
        FLUSSPFERD_CD_PROPERTIES(p_constructor_properties) \
        BOOST_PP_EXPR_IF( \
          p_augment_constructor, \
          Class :: augment_constructor(obj);) \
      } \
      typedef boost::mpl::bool_< (p_custom_enumerate) > custom_enumerate; \
    }; \
    BOOST_PP_REPEAT( \
      BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT), \
      FLUSSPFERD_CD_CTOR_FWD, \
      (BOOST_PP_CAT(p_cpp_name, _base), p_base)) \
  }; \
  class p_cpp_name \
  : \
    public BOOST_PP_CAT(p_cpp_name, _base) < p_cpp_name > \
  /* */

#define FLUSSPFERD_CD_CTOR_FWD(z, n, d) \
  BOOST_PP_EXPR_IF(n, template<) \
  BOOST_PP_ENUM_PARAMS(n, typename P) \
  BOOST_PP_EXPR_IF(n, >) \
  BOOST_PP_TUPLE_ELEM(2, 0, d) \
  ( \
    BOOST_PP_ENUM_BINARY_PARAMS(n, P, const &p) \
  ) \
  : \
    BOOST_PP_TUPLE_ELEM(2, 1, d) \
    ( \
      BOOST_PP_ENUM_PARAMS(n, p) \
    ) \
  { } \
  /* */

#define FLUSSPFERD_CD_METHODS(p_methods) \
  BOOST_PP_SEQ_FOR_EACH( \
    FLUSSPFERD_CD_METHOD, \
    ~, \
    FLUSSPFERD_PP_GEN_TUPLE3SEQ(p_methods)) \
  /* */

#define FLUSSPFERD_CD_METHOD(r, d, p_method) \
  BOOST_PP_CAT( \
    FLUSSPFERD_CD_METHOD__, \
    BOOST_PP_TUPLE_ELEM(3, 1, p_method) \
  ) ( \
    BOOST_PP_TUPLE_ELEM(3, 0, p_method), \
    BOOST_PP_TUPLE_ELEM(3, 2, p_method) \
  ) \
  /* */

#define FLUSSPFERD_CD_METHOD__bind(p_method_name, p_bound) \
  ::flusspferd::create_native_method( \
      obj, \
      (p_method_name), \
      & Class :: p_bound); \
  /* */

#define FLUSSPFERD_CD_METHOD__bind_static(p_method_name, p_bound) \
  ::flusspferd::create_native_function( \
      obj, \
      (p_method_name), \
      & Class :: p_bound); \
  /* */

#define FLUSSPFERD_CD_METHOD__alias(p_method_name, p_alias) \
  obj.define_property( \
    (p_method_name), \
    obj.get_property((p_alias)), \
    ::flusspferd::dont_enumerate); \
  /* */

#define FLUSSPFERD_CD_METHOD__none(p_method_name, p_param) \
  /* */

#define FLUSSPFERD_CD_PROPERTIES(p_properties) \
  BOOST_PP_SEQ_FOR_EACH( \
    FLUSSPFERD_CD_PROPERTY, \
    ~, \
    FLUSSPFERD_PP_GEN_TUPLE3SEQ(p_properties)) \
  /* */

#define FLUSSPFERD_CD_PROPERTY(r, d, p_property) \
  BOOST_PP_CAT( \
    FLUSSPFERD_CD_PROPERTY__, \
    BOOST_PP_TUPLE_ELEM(3, 1, p_property) \
  ) ( \
    BOOST_PP_TUPLE_ELEM(3, 0, p_property), \
    BOOST_PP_TUPLE_ELEM(3, 2, p_property) \
  ) \
  /* */

#define FLUSSPFERD_CD_PROPERTY__getter_setter(p_property_name, p_param) \
  obj.define_property( \
    (p_property_name), \
    ::flusspferd::property_attributes( \
      ::flusspferd::permanent_shared_property, \
      ::flusspferd::create_native_method( \
        ::flusspferd::object(), \
        "$get_" p_property_name, \
        & Class :: \
        BOOST_PP_TUPLE_ELEM(2, 0, p_param) \
      ), \
      ::flusspferd::create_native_method( \
        ::flusspferd::object(), \
        "$set_" p_property_name, \
        & Class :: \
        BOOST_PP_TUPLE_ELEM(2, 1, p_param) \
      ) \
    ) \
  ); \
  /* */

#define FLUSSPFERD_CD_PROPERTY__getter(p_property_name, p_param) \
  obj.define_property( \
    (p_property_name), \
    ::flusspferd::property_attributes( \
      ::flusspferd::permanent_shared_property \
      | ::flusspferd::read_only_property, \
      ::flusspferd::create_native_method( \
        ::flusspferd::object(), \
        "$get_" p_property_name, \
        & Class :: p_param \
      ) \
    ) \
  ); \
  /* */

#define FLUSSPFERD_CD_PROPERTY__constant(p_property_name, p_param) \
  obj.define_property( \
    (p_property_name), \
    ::flusspferd::value((p_param)), \
    ::flusspferd::read_only_property | ::flusspferd::permanent_property); \
  /* */

#define FLUSSPFERD_CD_PROPERTY__variable(p_property_name, p_param) \
  obj.define_property( \
    (p_property_name), \
    ::flusspferd::value((p_param))); \
  /* */

#define FLUSSPFERD_CD_PROPERTY__none(p_property_name, p_param) \
  /* */

#define FLUSSPFERD_CLASS_DESCRIPTION(p_cpp_name, tuple_seq) \
  FLUSSPFERD_CLASS_DESCRIPTION_A( \
    FLUSSPFERD_CD_PARAM( \
      tuple_seq \
      (cpp_name, p_cpp_name) \
    ) \
  ) \
  /* */

#else // IN_DOXYGEN

/**
 * Generate a @ref flusspferd::load_class "loadable" class named @p cpp_name.
 *
 * Also generates a template class <code>cpp_name<em>_base</em>&lt;T&gt;</code>.
 * This class contains a typedef @c base_type to itself, and it is the direct
 * base class of @p cpp_name. This implies that inside @p cpp_name, the
 * identifier @p base_type can be used to reference the direct base class,
 * especially in constructors.
 *
 * The class @p base_type has forwarding constructors to its base class (by
 * default flusspferd::native_object_base), taking each parameter as a constant
 * reference to its type. These forwarding constructors can be used to
 * initialize the real (indirect) base class.
 *
 * Most importantly, @p base_type contains a class @c class_info, with all
 * elements set according to the named parameters.
 *
 * <dl><dt><b>Usage template:</b></dt><dd>
 * @code
FLUSSPFERD_CLASS_DESCRIPTION(
    cpp_name,
    (parameter_name_1, parameter_value_1)
    (parameter_name_2, parameter_value_2)
    ...
)
{
    CONTENTS
};
 * @endcode
 * </dd></dl>
 *
 * @param cpp_name The name of the generated class.
 * @param named_parameters A sequence of named parameters in the form
 *                 <code>(parameter1_name, parameter2_value)
 *                 (parameter2_name, parameter2_value) ...</code>
 *
 * <dl><dt><b>Named parameters:</b></dt>
 * <dd><dl>
 * <dt><em>base</em> (optional)</dt>
 * <dd><b>{Class}</b> The (indirect) base class. Must be derived from
 *     flusspferd::native_object_base and contain a valid class_info.
 *     The class' prototype will be used as the prototype of the generated
 *     prototype (i.e., @c instanceof works).
 *     <br>Default: flusspferd::native_object_base.
 * </dd>
 * <dt><em>constructor_name</em> (required)</dt>
 * <dd><b>{String}</b> The name of the constructor inside the container passed
 *     to flusspferd::load_class.</dd>
 * <dt><em>constructor_arity</em> (optional)</dt>
 * <dd><b>{Integer}</b> The Javascript constructor's arity.
 *     <br>Default: @c 0.</dd>
 * <dt><em>constructible</em> (optional)</dt>
 * <dd><b>{Boolean}</b> Whether the class is constructible from Javascript.
 *     <br>Default: @c true.</dd>
 * <dt><em>full_name</em> (required)</dt>
 * <dd><b>{String}</b> The full, identifying name of the class, must be
 *     system-unique.</dd>
 * <dt><em>methods</em> (optional)</dt>
 * <dd><b>[3-tuple Sequence]</b> Sequence of automatically generated Javascript
 *     methods. Of the form <code>(method_name, type, parameter)...</code>,
 *     where @c method_name is a string.
 *     <br>Default: <code>(~, none, ~)</code></dd>
 * <dt><em>constructor_methods</em> (optional)</dt>
 * <dd><b>[3-tuple Sequence]</b> Sequence of automatically generated Javascript
 *     methods <b>on the constructor</b>. Of the form <code>(method_name, type,
 *     parameter)...</code>,
 *     where @c method_name is a string.
 *     <br>Default: <code>(~, none, ~)</code></dd>
 * <dt><em>properties</em> (optional)</dt>
 * <dd><b>[3-tuple Sequence]</b> Sequence of automatically generated Javascript
 *     properties. Of the form <code>(property_name, type, parameter)...</code>,
 *     where @c property_name is a string.
 *     <br>Default: <code>(~, none, ~)</code></dd>
 * <dt><em>constructor_properties</em> (optional)</dt>
 * <dd><b>[3-tuple Sequence]</b> Sequence of automatically generated Javascript
 *     properties <b>on the constructor</b>. Of the form <code>(property_name,
 *     type, parameter)...</code>,
 *     where @c property_name is a string.
 *     <br>Default: <code>(~, none, ~)</code></dd>
 * <dt><em>custom_enumerate</em> (optional)</dt>
 * <dd><b>{Boolean}</b> Whether the class overrides the standard enumerate
 *     hooks. (Enables class_info::custom_enumerate.)
 *     <br>Default: @c true.</dd>
 * <dt><em>augment_constructor</em> (optional)</dt>
 * <dd><b>{0,1}</b> If set to 1, @p cpp_name::augment_constructor(ctor) will be
 *     called with the constructor object as parameter when the Javascript
 *     constructor is being generated by flusspferd::load_class (or more
 *     specifically, @c base_type::class_info::augment_constructor).
 *     <br>Default: 0</dd>
 * <dt><em>augment_prototype</em> (optional)</dt>
 * <dd><b>{0,1}</b> If set to 1, @p cpp_name::augment_prototype(ctor) will be
 *     called with the prototype object as parameter when the Javascript
 *     constructor is being generated by flusspferd::load_class (or more
 *     specifically, @c base_type::class_info::create_prototype).
 *     <br>Default: 0</dd>
 * </dl></dd></dl>
 *
 * <dl><dt><b>Method types:</b></dt>
 * <dd>The parameters @c methods and @c constructor_methods take sequences with elements of
 *     the form <code>(method_name, <b>type</b>, parameter)</code>, where @c type is one of
 *     the identifiers mentioned below.
 *     <br><br>
 *     <dl>
 *     <dt><code>(?, <b>none</b>, ?)</code></dt>
 *     <dd>Generates no method. Used as a dummy of the form <code>(~, none, ~)</code>
 *         because empty sequences are not valid.</dd>
 *     <dt><code>(name, <b>bind</b>, method)</code></dt>
 *     <dd>Binds the method with name @p name to non-static method @c cpp_name::method.</dd>
 *     <dt><code>(name, <b>bind_static</b>, method)</code></dt>
 *     <dd>Binds the method with name @p name to the static method @c cpp_name::method.</dd>
 *     <dt><code>(name, <b>alias</b>, alias_name)</code></dt>
 *     <dd>Copies the method @p alias_name into a property with name @p name. The method
 *         @p alias_name must be already defined @em above this method.</dd>
 *     </dl>
 * </dd></dl>
 *
 * <dl><dt><b>Property types:</b></dt>
 * <dd>The parameters @c properties and @c constructor_properties take sequences with elements
 *     of the form <code>(property_name, <b>type</b>, parameter)</code>, where @c type is one
 *     of the identifiers mentioned below.
 *     <br><br>
 *     <dl>
 *     <dt><code>(?, <b>none</b>, ?)</code></dt>
 *     <dd>Generates no property. Used as a dummy of the form <code>(~, none, ~)</code>
 *         because empty sequences are not valid.</dd>
 *     <dt><code>(name, <b>getter_setter</b>, (getter, setter))</code></dt>
 *     <dd>Generates a property that is accessed through the accessors (non-static methods)
 *         @p cpp_name::getter and @p cpp_name::setter.</dd>
 *     <dt><code>(name, <b>getter</b>, getter)</code></dt>
 *     <dd>Generates a @em constant property that is accessed through the non-static method
 *         @p cpp_name::getter.</dd>
 *     <dt><code>(name, <b>variable</b>, initial_value)</code></dt>
 *     <dd>Generates a standard property with the initial value @p initial_value.</dd>
 *     <dt><code>(name, <b>constant</b>, value)</code></dt>
 *     <dd>Generates a constant property with the value @p value.</dd>
 *     </dl>
 * </dd></dl>
 *
 * <dl><dt><b>Example:</b></dt>
 * <dd>
 * @code
FLUSSPFERD_CLASS_DESCRIPTION(
    my_class,
    (full_name, "MyModule.MyClass")
    (constructor_name, "MyClass")
    (methods,
        ("myMethod", bind, my_method)
        ("anotherName", alias, "myMethod"))
    (constructor_methods,
        ("constructorMethod", bind_static, constructor_method))
    (constructor_properties,
        ("VERSION", constant, flusspferd::string("1.0"))))
{
    double my_method(double parameter) {
        return parameter * 2;
    }
    static double constructor_method(double parameter1, int parameter2) {
        return parameter1 + parameter1;
    }
};

void some_function() {
    flusspferd::load_class<my_class>();
}
@endcode
 * </dd></dl>
 *
 * @see flusspferd::load_class, flusspferd::class_info
 *
 * @ingroup classes
 */
#define FLUSSPFERD_CLASS_DESCRIPTION(cpp_name, named_parameters) ...

#endif // IN_DOXYGEN

#endif
