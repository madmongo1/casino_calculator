#pragma once

#include "dealer_hand.hpp"
#include "player_hand.hpp"
#include "shoe.hpp"
#include "rules.hpp"
#include "outcome.hpp"
#include "score.hpp"
#include "polyfill/static_vector.hpp"
#include <cassert>


namespace blackjack {

enum class player_action
{
    hit,
    stick,
    double_down,
    split,
};

inline auto
operator<<(
    std::ostream &os,
    player_action pa) -> std::ostream &
{
    switch (pa)
    {
    case player_action::double_down: os << "double down";
        break;
    case player_action::hit: os << "hit";
        break;
    case player_action::stick: os << "stick";
        break;
    case player_action::split: os << "split";
        break;
    }

    return os;
}

struct scenario_result
    : outcome
{
    scenario_result(player_action action)
        : action(action)
        , outcome()
    {}

    player_action action;

    friend auto
    operator<<(
        std::ostream &os,
        scenario_result const &sr) -> std::ostream &
    {
        return os << sr.action << " : pays " << sr.payoff << " : prob " << sr.probability;
    }
};

struct scenario
{
    using result_vector = polyfill::static_vector<scenario_result, 4>;

    static scenario_result
    best_of(result_vector const &v)
    {
        assert(!v.empty());
        auto first = v.begin();
        auto last = v.end();

        auto result = *first++;
        while (first != last)
        {
            if (first->payoff > result.payoff)
                result = *first;
            ++first;
        }

        return result;
    }

    static auto hit_player(        rules const &r,
                                   shoe const& s,
                                   player_hand const& p,
                                   dealer_hand const &d) -> outcome
    {
        polyfill::static_vector<outcome, nof_card_scales> outcomes;
        int total_cards_in_shoe = 0;
        for (std::size_t i = 0; i < nof_card_scales; ++i)
        {
            auto avail = s[to_card_scale(i)];
            if (avail)
            {
                total_cards_in_shoe += avail;

                auto s2 = s;
                auto p2 = p;
                deal_one(s2, p2, to_card_scale(i));
                outcomes.push_back(dealers_turn(r, s2, score(p2), d) * double(avail));
            }
        }

        auto scale = 1.0 / double(total_cards_in_shoe);

        double pay = 0.0;
        for (auto &&o : outcomes)
        {
            pay += o.payoff * o.probability;
        }
        pay *= scale;

        return outcome{.payoff = pay, .probability = 1.0};
    }

    inline
    static auto
    run(
        rules const &r,
        shoe const &s,
        player_hand const &p,
        dealer_hand const &d)
    -> scenario_result
    {
        auto possible_results = polyfill::static_vector<scenario_result, 4>();
        if (r.may_stick(p))
        {
            auto &res = possible_results.push_back(scenario_result(player_action::stick));
            res.update(dealers_turn(r, s, score(p), d));
        }
        if (r.may_hit(p))
        {
            auto& res = possible_results.push_back(scenario_result(player_action::hit));
            res.update(hit_player(r, s, p, d));
        }
        /*
        if (r.may_double(p))
            possible_results.push_back(scenario_result(player_action::double_down));
        if (r.may_split(p))
            possible_results.push_back(scenario_result(player_action::split));
*/

        return best_of(possible_results);

    }

    inline static auto
    deal_one(
        shoe &s,
        hand &d,
        card_scale cs)
    -> void
    {
        s[cs] -= 1;
        d[cs] += 1;
    }

    inline static auto
    dealers_turn(
        rules const &r,
        shoe const &s,
        score const &player_score,
        dealer_hand const &d) -> outcome
    {
        auto accumulated_deal_one = [&] {
            polyfill::static_vector<outcome, nof_card_scales> outcomes;
            int total_cards_in_shoe = 0;
            for (std::size_t i = 0; i < nof_card_scales; ++i)
            {
                auto avail = s[to_card_scale(i)];
                if (avail)
                {
                    total_cards_in_shoe += avail;

                    auto s2 = s;
                    auto d2 = d;
                    deal_one(s2, d2, to_card_scale(i));
                    outcomes.push_back(dealers_turn(r, s2, player_score, d2) * double(avail));
                }
            }

            auto scale = 1.0 / double(total_cards_in_shoe);

            double pay = 0.0;
            for (auto &&o : outcomes)
            {
                pay += o.payoff * o.probability;
            }
            pay *= scale;

            return outcome{.payoff = pay, .probability = 1.0};
        };

        auto dealer_score = score(d);
        switch (r.select_dealer_action(dealer_score))
        {
        case dealer_action::hit:
        {
            return accumulated_deal_one();
        }
        case dealer_action::stand:
        {
            return outcome{.payoff = r.payoff(player_score, dealer_score), .probability = 1.0};
        }
        }
        assert(!"logic error");
        return outcome();
    }

};

}