#include <blackjack/scenario.hpp>
#include <iostream>

#include "blackjack/rules.hpp"
#include "blackjack/shoe.hpp"

namespace blackjack {
outcome check(scenario &s, shoe const &sh, player_hand const &p,
              dealer_hand const &d) {
  auto result = s.run(sh, p, d);
  std::cout << "player has " << p << " vs dealer's " << d << " player should "
            << result << '\n';
  return result;
}

void iterate_all(scenario &s, outcome &accum) {
  auto sh = blackjack::shoe(1);

  auto one = [&](card_scale p1, card_scale p2, card_scale d) {
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

  for (std::size_t p1 = 0; p1 < blackjack::nof_card_scales; ++p1) {
    for (std::size_t p2 = 0; p2 < blackjack::nof_card_scales; ++p2) {
      for (std::size_t d = 0; d < blackjack::nof_card_scales; ++d) {
        accum += one(to_card_scale(p1), to_card_scale(p2), to_card_scale(d));
      }
    }
  }
}

} // namespace blackjack

int main() {
  auto rules = blackjack::rules();
  auto scenario = blackjack::scenario(rules);
  auto accum = blackjack::outcome(0, 0);
  blackjack::iterate_all(scenario, accum);

  std::cout << "overall payoff: " << accum << std::endl;

  return 0;
}
