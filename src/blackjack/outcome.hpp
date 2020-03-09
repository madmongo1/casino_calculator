#pragma once

namespace blackjack {

struct outcome
{
    void update(outcome const& other) noexcept
    {
        *this = other;
    }

    outcome& operator*=(double prob)
    {
        probability *= prob;
        return *this;
    }

    double payoff = 0.0;
    double probability = 1.0;
};

inline
outcome operator*(outcome l, double prob)
{
    l *= prob;
    return l;
}

}