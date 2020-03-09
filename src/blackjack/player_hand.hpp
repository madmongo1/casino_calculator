#pragma once

#include "hand.hpp"
#include <optional>

namespace blackjack {

struct player_hand
    : hand
{
    using hand::hand;

    std::optional<card_scale>
    is_pair() const
    {
        std::optional<card_scale> result;
        int total_cards = 0;
        for (std::size_t i = 0; i < store_.size(); ++i)
        {
            auto count = store_[i];
            if (count)
            {
                total_cards += count;
                if (count == 2)
                    result = to_card_scale(i);
            }
        }
        if (total_cards != 2)
            result.reset();
        return result;
    }
};

}