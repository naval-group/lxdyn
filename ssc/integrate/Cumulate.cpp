/*
 * Cumulate.cpp
 *
 * \date 8 oct. 2013, 08:22:31
 * \author cec
 */

#include "Cumulate.hpp"
#include "ssc/pimpl_idiom/generic_pimpl_implementation.hpp"
#include <list>

class ssc::integrate::Cumulate::Impl
{
    public:
        Impl() : x_y_pairs(std::list<std::pair<double,double> >())
        {

        }

        void add(const double x, const double y)
        {
            x_y_pairs.push_back(std::make_pair(x,y));
            x_y_pairs.sort();
        }

        double integrate(const double a, const double b) const
        {
            if (x_y_pairs.size()<2)
            {
                return 0;
            }
            double ret = 0;
            for (auto it1 = x_y_pairs.begin() ; it1 != x_y_pairs.end() ; ++it1)
            {
                auto it2 = it1;
                if (++it2 != x_y_pairs.end())
                {
                    double x1 = it1->first;
                    double x2 = it2->first;
                    double y1 = it1->second;
                    double y2 = it2->second;
                    if ((it1->first < a) && (it2->first >= a))
                    {
                        y1 = (y2-y1)/(x2-x1)*(a-x1)+y1;
                        x1 = a;
                    }
                    if ((it1->first < b)&&(it2->first >= b))
                    {
                        y2 = (y2-y1)/(x2-x1)*(b-x2)+y2;
                        x2 = b;
                    }
                    if ((it1->first<b) && (it2->first>=a))
                    {
                        ret += (x2-x1)*(y1+y2)/2;
                    }
                }
            }
            return ret;
        }

    private:
        std::list<std::pair<double,double> > x_y_pairs;

};


ssc::integrate::Cumulate::Cumulate() : pimpl{Impl{}}
{

}

ssc::integrate::Cumulate::~Cumulate()
{

}

void ssc::integrate::Cumulate::add(const double x, const double y)
{
    pimpl->add(x,y);
}

double ssc::integrate::Cumulate::integrate(const double a, const double b)
{
    return pimpl->integrate(a,b);
}
