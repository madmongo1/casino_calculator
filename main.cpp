#include <blackjack/scenario.hpp>
#include <iostream>

#include "blackjack/rules.hpp"
#include "blackjack/shoe.hpp"
#include <boost/algorithm/string.hpp>

namespace blackjack {
outcome
check(
    scenario &s,
    shoe const &sh,
    player_hand const &p,
    dealer_hand const &d)
{
    auto result = s.run(sh, p, d, cards());
    std::cout << "player has " << p << " vs dealer's " << d << " player should "
              << result << '\n';
    return result;
}

void
iterate_all(
    scenario &s,
    outcome &accum)
{
    auto sh = blackjack::shoe(1);

    auto one = [&](
        card_scale p1,
        card_scale p2,
        card_scale d) {
        auto player = player_hand(p1, p2);
        auto dealer = dealer_hand(d);
        auto prob_comp = draw_probability(sh);
        auto prob =
            prob_comp.update(p1) * prob_comp.update(p2) * prob_comp.update(d);
        sh -= p1;
        sh -= p2;
        sh -= d;
        auto result = check(s, sh, player, dealer) * prob;
        sh += p1;
        sh += p2;
        sh += d;
        return result;
    };

    //  one(card_scale::ten, card_scale::ace, card_scale::nine);

    for (auto p1 : all_card_faces())
        for (auto p2 : all_card_faces())
            for (auto d : all_card_faces())
                accum += one(to_card_scale(p1), to_card_scale(p2), to_card_scale(d));
}

void play(rules const& r)
{
    auto s = scenario(r);
    auto burn_pile = blackjack::cards();
    auto dealer_shoe = blackjack::shoe(r.no_of_decks, r.cards_behind_cut);

    std::cout << r << std::endl;

    auto player = player_hand();
    auto dealer = dealer_hand();

    while(1)
    {
        std::cout << "enter command, ? for help: " << std::flush;
        std::string command;
        std::cin >> command;
        boost::trim(command);
        if (command == "?")
        {
            std::cout << "available commands:"
                         "\n  ps = peek into the dealer's shoe"
                         "\n  pd = peek into the discard pile"
                         "\n  play = play a random hand"
                         "\n  why = ask for an explanation of the play suggestion"
                         "\n";
            continue;
        }
        else if(boost::iequals(command, "ps"))
        {
            std::cout << dealer_shoe << std::endl;
            continue;
        }
        else if(boost::iequals(command, "pd"))
        {
            std::cout << burn_pile << std::endl;
        }
        else if(boost::iequals(command, "play"))
        {
            burn_pile += std::move(dealer);
            burn_pile += std::move(player);
            auto deal_to = [&](auto&& prefix, cards& hand)
            {
                std::cout << prefix;
                if (dealer_shoe.exhausted())
                {
                    std::cout << "reshuffle!...";
                    dealer_shoe += std::move(burn_pile);
                }
                auto card = dealer_shoe.select_random_card(std::cout);
                dealer_shoe -= card;
                hand += card;
                std::cout << std::endl;
            };

            deal_to("player: ", player);
            deal_to("dealer: ", dealer);
            deal_to("player: ", player);

            std::cout << "player has: " << player << ", dealer has: " << dealer << std::endl;
            std::cout << "suggested play: " << s.run(dealer_shoe, player, dealer, burn_pile) << std::endl;
        }
        else if (boost::iequals(command, "why"))
        {
            if (player.empty())
                std::cout << "why what?\n";
            else
            {
                s.chat(&std::cout);
                s.run(dealer_shoe, player, dealer, burn_pile);
                s.chat(nullptr);
            }
        }
    }
}

} // namespace blackjack

int
main()
{
    auto rules = blackjack::rules();
    blackjack::play(rules);
    auto scenario = blackjack::scenario(rules);
    auto burn_pile = blackjack::cards();
    auto shoe = blackjack::shoe(rules.no_of_decks, rules.cards_behind_cut);

    auto accum = blackjack::outcome(0, 0);
    blackjack::iterate_all(scenario, accum);

    std::cout << "overall payoff: " << accum << std::endl;

    return 0;
}
