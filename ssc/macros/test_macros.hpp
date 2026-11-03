/*
 * File:   test_macros.hpp
 * Author: leroux
 *
 * Created on August 22, 2011, 3:44 PM
 */

#ifndef TEST_MACROS_HPP
#define TEST_MACROS_HPP
#include <iostream>

#define QUOTE_ME(x) #x
#define QUOTE(x) QUOTE_ME(x)
#define COUT(val) std::cout << "in file " << __FILE__ << ":" << __LINE__ << " " << #val << " = " << val << std::endl;
#define CERR(val) std::cerr << "in file " << __FILE__ << ":" << __LINE__ << " " << #val << " = " << val << std::endl;

#endif /* TEST_MACROS_HPP */
