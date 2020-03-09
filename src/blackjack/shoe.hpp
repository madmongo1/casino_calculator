#pragma once

#include "cards.hpp"

namespace blackjack {

struct shoe
    : cards
{
    shoe(std::size_t decks = 1)
    : cards()
    {
        add(card_scale::two, 4 * decks);
        add(card_scale::three, 4 * decks);
        add(card_scale::four, 4 * decks);
        add(card_scale::five, 4 * decks);
        add(card_scale::six, 4 * decks);
        add(card_scale::seven, 4 * decks);
        add(card_scale::eight, 4 * decks);
        add(card_scale::nine, 4 * decks);
        add(card_scale::ten, 16 * decks);
        add(card_scale::ace, 4 * decks);
    }

};

}