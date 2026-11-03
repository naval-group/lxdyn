/*
 * generic_pimpl.hpp
 *
 * \date 8 oct. 2013, 09:02:01
 * \author cec
 */

#ifndef GENERIC_PIMPL_HPP_
#define GENERIC_PIMPL_HPP_

// From http://herbsutter.com/gotw/_101/

#include <memory>

template<typename T> class Pimpl
{
    private:
        std::unique_ptr<T> m;
    public:
        Pimpl();
        template<typename ...Args> Pimpl( Args&& ... );
        ~Pimpl();
        T* operator->();
        T& operator*();
};

#endif /* GENERIC_PIMPL_HPP_ */
