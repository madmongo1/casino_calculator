#include <iostream>
#include <blackjack/scenario.hpp>

#include "blackjack/rules.hpp"
#include "blackjack/shoe.hpp"

namespace blackjack {
void
check(
    rules const &r,
    shoe const &s,
    player_hand
    const &p,
    dealer_hand const &d
)
{
    std::cout << "player has " << p << " vs dealer's " << d << " player should " << blackjack::scenario::run(r, s, p, d)
              << '\n';
}

void iterate_all(rules const& r)
{
    auto s = blackjack::shoe(1);
    for (std::size_t p1 = 0 ; p1 < blackjack::nof_card_scales ; ++p1)
    {
        for (std::size_t p2 = 0 ; p2 < blackjack::nof_card_scales ; ++p2)
        {
            for (std::size_t d = 0 ; d < blackjack::nof_card_scales ; ++d)
            {
                auto player = player_hand(blackjack::to_card_scale(p1), to_card_scale(p2));
                auto dealer = dealer_hand(blackjack::to_card_scale(d));
                --s[to_card_scale(p1)];
                --s[to_card_scale(p2)];
                --s[to_card_scale(d)];
                check(r, s, player, dealer);
                ++s[to_card_scale(p1)];
                ++s[to_card_scale(p2)];
                ++s[to_card_scale(d)];
            }

        }

    }

}

}

int
main()
{
    auto rules = blackjack::rules();

    blackjack::iterate_all(rules);

    return 0;
}
