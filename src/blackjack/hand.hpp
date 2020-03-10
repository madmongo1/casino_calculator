#pragma once

#include "cards.hpp"

namespace blackjack {

struct hand
    : cards
{
    using cards::cards;

    friend std::string const& to_string(hand const& h)
    {
        static thread_local std::string rep;

        rep.clear();
        for(std::size_t i = nof_card_scales ; i-- ; )
        {
            auto card = to_card_scale(i);
            auto count = h[card];
            while(count--)
                rep += to_char(card);
        }
        return rep;
    }

    friend std::ostream& operator<<(std::ostream& os, hand const& h)
    {
        return os << to_string(h);
    }
};

}
