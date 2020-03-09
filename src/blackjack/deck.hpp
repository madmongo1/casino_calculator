#pragma once

namespace project
{


struct shoe : cards
{
    shoe(int decks = 1)
    : cards()
    {
        for (int i = 0 ; i < nof_card_scales ; ++i)
        {
            if (i == to_int(card_scale::ten))
            {
                cards_[i] = 4 * 4 * i;
            }
            else
            {
                cards_[i] = 4 * i;
            }
        }
    }
};


struct hand : cards
{
    score
    compute_score() const
    {
        if (count(card_scale::ace) == 1 and count(card_scale::ten) == 1)
        {
            return score { .value_ = 21, .soft_ = false, .blackjack_ = true };
        }

    }
};



}
