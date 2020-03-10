#pragma once
#include <ostream>
#include <iomanip>

namespace polyfill
{

struct percentage
{
    double value;
    int precision;

    percentage(double value, int precision = 4)
    : value(value), precision(precision)
    {}

    friend std::ostream& operator<<(std::ostream& os, percentage const& p)
    {
        auto flags = os.flags();
        os << std::setprecision(p.precision) << (p.value * 100) << '%';
        os.flags(flags);
        return os;
    }

};

}