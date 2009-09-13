// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld,
                         Vinzenz Feenstra

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
#ifndef FLUSSPFERD_CONFIG_HPP
#define FLUSSPFERD_CONFIG_HPP

#include <boost/preprocessor/control/if.hpp>

#define FLUSSPFERD_PRIVATE_API

#if defined(_MSC_VER) && (defined(WIN32) || defined(WIN64))
#   ifndef FLUSSPFERD_PUBLIC_API
#       ifdef FLUSSPFERD_BUILD_SHARED
#           define FLUSSPFERD_PUBLIC_API __declspec(dllexport)
#           define FLUSSPFERD_PUBLIC_MODULE_API __declspec(dllexport)
#           define FLUSSPFERD_TEMPLATE_IMPORT
#           define FLUSSPFERD_PROPERTY_ITERATOR_NEEDS_DECREMENT
#       elif defined(FLUSSPFERD_SHARED)
#           define FLUSSPFERD_PUBLIC_API __declspec(dllimport)
#           define FLUSSPFERD_PUBLIC_MODULE_API __declspec(dllimport)
#           define FLUSSPFERD_TEMPLATE_IMPORT extern
#           define FLUSSPFERD_PROPERTY_ITERATOR_NEEDS_DECREMENT
#       endif 
#   endif
#   define FLUSSPFERD_LOADER_API __declspec(dllexport)
#endif

#ifndef FLUSSPFERD_PUBLIC_API
#   define FLUSSPFERD_PUBLIC_API
#endif 

#ifndef FLUSSPFERD_PUBLIC_MODULE_API 
#   define FLUSSPFERD_PUBLIC_MODULE_API 
#endif

#ifndef FLUSSPFERD_PRIVATE_MODULE_API 
#   define FLUSSPFERD_PRIVATE_MODULE_API 
#endif

#ifndef FLUSSPFERD_LOADER_API
#   define FLUSSPFERD_LOADER_API 
#endif

#define FLUSSPFERD_PUBLIC_API_CLASS  1
#define FLUSSPFERD_PRIVATE_API_CLASS 0

#define FLUSSPFERD_API_CLASS_PUBLIC_()  FLUSSPFERD_PUBLIC_MODULE_API
#define FLUSSPFERD_API_CLASS_PRIVATE_() FLUSSPFERD_PRIVATE_MODULE_API

#define FLUSSPFERD_API_CLASS( API_TYPE ) \
    BOOST_PP_IF( API_TYPE , FLUSSPFERD_API_CLASS_PUBLIC_, FLUSSPFERD_API_CLASS_PRIVATE_)()

#ifdef FLUSSPFERD_PROPERTY_ITERATOR_NEEDS_DECREMENT
#   undef FLUSSPFERD_PROPERTY_ITERATOR_NEEDS_DECREMENT
#   define FLUSSPFERD_PROPERTY_ITERATOR_NEEDS_DECREMENT \
    void decrement(){ \
        throw std::logic_error(\
            "flusspferd::property_iterator is a forward "\
            "iterator you cannot move backwards"\
        );\
    }
#else
#   define FLUSSPFERD_PROPERTY_ITERATOR_NEEDS_DECREMENT
#endif

#endif //FLUSSPFERD_CONFIG_HPP
