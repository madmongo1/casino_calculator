#pragma once

#include "cards.hpp"
#include "polyfill/percent.hpp"
#include <random>
#include <cassert>

namespace blackjack {

struct shoe
    : cards
{
    shoe(
        std::size_t decks = 1,
        int cards_behind_cut = 0)
        : cards()
        , cards_behind_cut(cards_behind_cut)
    {
        adjust(card_scale::two, 4 * decks);
        adjust(card_scale::three, 4 * decks);
        adjust(card_scale::four, 4 * decks);
        adjust(card_scale::five, 4 * decks);
        adjust(card_scale::six, 4 * decks);
        adjust(card_scale::seven, 4 * decks);
        adjust(card_scale::eight, 4 * decks);
        adjust(card_scale::nine, 4 * decks);
        adjust(card_scale::ten, 16 * decks);
        adjust(card_scale::ace, 4 * decks);
    }

    bool
    exhausted() const
    {
        return count() == cards_behind_cut;
    }

    double
    probability(card_scale c) const
    {
        return double(count(c)) / double(count());
    }

    int cards_behind_cut;

    friend std::ostream &
    operator<<(
        std::ostream &os,
        shoe const &s)
    {
        const char *sep = "";
        for (auto c : all_card_faces())
        {
            os << sep << c << " : " << s.count(c) << " (" << polyfill::percentage(s.probability(c)) << ')';
            sep = "\n";
        }
        os << "\ncards behind cut      : " << s.cards_behind_cut
           << "\ncards until exhausted : " << (s.count() - s.cards_behind_cut);
        return os;
    }

    card_scale select_random_card(std::ostream& logger) const
    {
        auto make_device = []
        {
            auto rnd = std::random_device();
            std::seed_seq seq { rnd(), rnd(), rnd(), rnd(), rnd() };
            return std::default_random_engine(seq);
        };
        thread_local static std::default_random_engine eng = make_device();

        auto max = count();
        auto dist = std::uniform_int_distribution<int>(0, count() - 1);
        auto rv = dist(eng);
        for (auto card : all_card_faces())
        {
            if (auto cc = count(card))
            {
                if (rv < cc)
                {
                    logger << "selected " << card << " (" << polyfill::percentage(probability(card)) << ')';
                    return card;
                }
                rv -= cc;
            }
        }
        assert(!"logic error");
        return card_scale ::ace;
    }
};

}