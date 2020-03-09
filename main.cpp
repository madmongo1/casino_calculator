#include <iostream>
#include <blackjack/scenario.hpp>

#include "blackjack/rules.hpp"
#include "blackjack/shoe.hpp"

namespace blackjack {
void
check(
    scenario &s,
    shoe const &sh,
    player_hand
    const &p,
    dealer_hand const &d
)
{
    std::cout << "player has " << p << " vs dealer's " << d << " player should " << s.run(sh, p, d)
              << '\n';
}

void
iterate_all(scenario &s)
{
    auto sh = blackjack::shoe(1);
    for (std::size_t p1 = 0; p1 < blackjack::nof_card_scales; ++p1)
    {
        for (std::size_t p2 = 0; p2 < blackjack::nof_card_scales; ++p2)
        {
            for (std::size_t d = 0; d < blackjack::nof_card_scales; ++d)
            {
                auto player = player_hand(blackjack::to_card_scale(p1), to_card_scale(p2));
                auto dealer = dealer_hand(blackjack::to_card_scale(d));
                --sh[to_card_scale(p1)];
                --sh[to_card_scale(p2)];
                --sh[to_card_scale(d)];
                check(s, sh, player, dealer);
                ++sh[to_card_scale(p1)];
                ++sh[to_card_scale(p2)];
                ++sh[to_card_scale(d)];
            }

        }

    }

}

}

int
main()
{
    auto rules = blackjack::rules();
    auto scenario = blackjack::scenario(rules);
    blackjack::iterate_all(scenario);

    return 0;
}
