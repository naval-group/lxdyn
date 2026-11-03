/*
 * smart_pointers_macros.hpp
 *
 * \date Jun 12, 2014
 * \author cec
 */

#ifndef TR1_MACROS_HPP_
#define TR1_MACROS_HPP_

#if defined(__clang__)
    /* Clang/LLVM. ---------------------------------------------- */
    #define TR1INC(x) <x>
    #define TR1(x) std::x
    #define TR1NAMESPACE 0
#elif defined(__ICC) || defined(__INTEL_COMPILER)
    /* Intel ICC/ICPC. ------------------------------------------ */
    #define TR1INC(x) <tr1/x>
    #define TR1(x) std::tr1::x
    #define TR1NAMESPACE 1
#elif defined(__GNUC__) || defined(__GNUG__)
    /* GNU GCC/G++. --------------------------------------------- */
    #define TR1INC(x) <tr1/x>
    #define TR1(x) std::tr1::x
    #define TR1NAMESPACE 1
#elif defined(__HP_cc) || defined(__HP_aCC)
    /* Hewlett-Packard C/aC++. ---------------------------------- */
    #define TR1INC(x) <tr1/x>
    #define TR1(x) std::tr1::x
    #define TR1NAMESPACE 1
#elif defined(__IBMC__) || defined(__IBMCPP__)
    /* IBM XL C/C++. -------------------------------------------- */
    #define TR1INC(x) <tr1/x>
    #define TR1(x) std::tr1::x
    #define TR1NAMESPACE 1
#elif defined(_MSC_VER)
    /* Microsoft Visual Studio. --------------------------------- */
    #define TR1INC(x) <x>
    #define TR1(x) std::tr1::x
    #define TR1NAMESPACE 0
    #define __PRETTY_FUNCTION__ __FUNCSIG__
#elif defined(__PGI)
    /* Portland Group PGCC/PGCPP. ------------------------------- */
    #define TR1INC(x) <tr1/x>
    #define TR1(x) std::tr1::x
    #define TR1NAMESPACE 1
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
    /* Oracle Solaris Studio. ----------------------------------- */
    #define TR1INC(x) <tr1/x>
    #define TR1(x) std::tr1::x
    #define TR1NAMESPACE 1
#endif

#endif /* TR1_MACROS_HPP_ */
