#pragma once

#include "cards.hpp"

namespace blackjack
{

struct score
{
    score(cards const& cards_)
        : value_(0)
        , soft_(false)
        , blackjack_(false)
    {
        if (cards_[card_scale::ace] == 1 and cards_[card_scale::ten] == 1)
        {
            value_ = 21;
            blackjack_ = true;
            return;
        }

        auto current_value = 2;
        for (auto&& ncards : cards_)
        {
            value_ += current_value++ * ncards;
        }
        if (cards_[card_scale::ace])
            soft_ = true;
    }

    int value() const {
        return value_ - (soft_ ? 10 : 0);
    }

    bool bust() const
    {
        return value() > 21;
    }

    bool soft() const {
        return soft_ and not bust();
    }

    bool blackjack() const {
        return blackjack_;
    }

private:
    int value_;
    bool soft_;
    bool blackjack_;
};

}