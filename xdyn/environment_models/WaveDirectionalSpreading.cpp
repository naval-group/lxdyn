/*
 * WaveDirectionalSpreading.cpp
 *
 *  Created on: Jul 31, 2014
 *      Author: cady
 */
#include "WaveDirectionalSpreading.hpp"
#include "SumOfWaveDirectionalSpreadings.hpp"

#include <algorithm>
#include <cmath>
#define PI M_PI


WaveDirectionalSpreading::WaveDirectionalSpreading() : psi0(0)
{
}

WaveDirectionalSpreading::WaveDirectionalSpreading(const double psi0_) : psi0(psi0_)
{
}

WaveDirectionalSpreading::~WaveDirectionalSpreading()
{
}

std::vector<std::pair<int,int> > WaveDirectionalSpreading::build_coprimes(const size_t n) const
{
    // Ternary tree of all coprime pairs, https://en.wikipedia.org/wiki/Coprime_integers
    std::vector<std::pair<int,int> > coprimes;
    std::vector<std::pair<int,int> > even_coprimes = {{2,1}};
    std::vector<std::pair<int,int> > odd_coprimes = {{3,1}};
    size_t even_index = 0;
    size_t odd_index = 0;
    bool take_even = true;
    while (coprimes.size() < n)
    {
        std::vector<std::pair<int,int> >& branch = take_even ? even_coprimes : odd_coprimes;
        size_t& index = take_even ? even_index : odd_index;
        const std::pair<int,int> current = branch.at(index);
        ++index;
        take_even = not take_even;
        const int m = current.first;
        const int p = current.second;
        branch.push_back(std::make_pair(2*m-p, m));
        branch.push_back(std::make_pair(m+2*p, p));
        branch.push_back(std::make_pair(2*m+p, m));
        for (size_t i = 0 ; i < 8 && coprimes.size() < n ; ++i)
        {
            coprimes.push_back(current);
        }
    }
    return coprimes;
}

std::vector<double> WaveDirectionalSpreading::get_directions(const size_t n,        //!< Number of angles to return
                                                             const bool periodic
                                                             ) const
{
    if (not periodic)
    {
        std::vector<double> psi(n, 0);
        const double two_pi = 2*PI;
        const double scale = two_pi/double(n);
        for (size_t i = 0 ; i < n ; ++i)
        {
            psi[i] = double(i)*scale;
        }
        return psi;
    }
    // The diagonals and the axes are periodic over a square domain whatever its size
    std::vector<double> psi {0, PI/4, PI/2, 3*PI/4, PI, 5*PI/4, 3*PI/2, 7*PI/4};
    const std::vector<std::pair<int,int> > coprimes = build_coprimes(n);
    size_t index = 0;
    while (psi.size() < n)
    {
        const std::pair<int,int> current = coprimes.at(8*index);
        const double m = current.first;
        const double p = current.second;
        const double hypotenuse = std::sqrt(m*m + p*p);
        const std::vector<double> octant {std::acos(p/hypotenuse), std::acos(m/hypotenuse),
                                          std::acos(-p/hypotenuse), std::acos(-m/hypotenuse),
                                          2*PI-std::acos(-m/hypotenuse), 2*PI-std::acos(-p/hypotenuse),
                                          2*PI-std::acos(m/hypotenuse), 2*PI-std::acos(p/hypotenuse)};
        for (size_t j = 0 ; j < 8 && psi.size() < n ; ++j)
        {
            psi.push_back(octant[j]);
        }
        ++index;
    }
    std::sort(psi.begin(), psi.end());
    return psi;
}

SumOfWaveDirectionalSpreadings WaveDirectionalSpreading::operator+(const WaveDirectionalSpreading& w) const
{
    return SumOfWaveDirectionalSpreadings(*this, w);
}
