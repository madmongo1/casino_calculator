#pragma once

#include "dealer_hand.hpp"
#include "outcome.hpp"
#include "player_hand.hpp"
#include "polyfill/static_vector.hpp"
#include "polyfill/universal.hpp"
#include "rules.hpp"
#include "score.hpp"
#include "shoe.hpp"
#include <cassert>
#include <ostream>
#include <iostream>

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
    case player_action::double_down:os << "double down";
        break;
    case player_action::hit:os << "hit";
        break;
    case player_action::stick:os << "stick";
        break;
    case player_action::split:os << "split";
        break;
    }

    return os;
}

struct scenario_result
    : outcome
{
    scenario_result(player_action action) : action(action)
        , outcome()
    {}

    player_action action;

    friend auto
    operator<<(
        std::ostream &os,
        scenario_result const &sr)
    -> std::ostream &
    {
        return os << sr.action << " : pays " << sr.payoff();
    }

};

struct scenario
{
    using result_vector = polyfill::static_vector<scenario_result, 4>;

    using player_key = std::tuple<player_hand, dealer_hand, shoe, cards /* burn pile */>;
    using player_memo_map = std::unordered_map<player_key, scenario_result, polyfill::universal_hash, polyfill::universal_equal_to>;

    using memo_key = std::tuple<score, dealer_hand, shoe, cards /* burn pile */>;
    using memo_map =
    std::unordered_map<memo_key, outcome, polyfill::universal_hash,
        polyfill::universal_equal_to>;

    scenario(rules const &r) : rules_(r)
    {}

    static scenario_result
    best_of(result_vector const &v)
    {
        assert(!v.empty());
        auto first = v.begin();
        auto last = v.end();

        auto result = *first++;
        while (first != last)
        {
            if (first->pnl() > result.pnl())
                result = *first;
            ++first;
        }

        return result;
    }

    auto
    hit_player(
        shoe s,
        player_hand const &p,
        dealer_hand const &d,
        cards burn_pile)
    -> outcome
    {
        if (s.exhausted())
        {
            chatter(context(), "shoe is exhausted so shuffle and lose count.");
            s += burn_pile;
            burn_pile.clear();
        }

        polyfill::static_vector<outcome, nof_card_scales> outcomes;
        int total_cards_in_shoe = 0;
        for (auto c : all_card_faces())
        {
            auto ctx = context(to_char(c));
            auto prob = s.probability(c);
            if (prob)
            {
                auto s2 = s;
                auto p2 = p;
                deal_one(s2, p2, c);
                auto scr = score(p2);
                chatter(ctx, "deal ", c, " with chance ", polyfill::percentage(prob), " scores ", scr);
                auto handle_score = [&]() -> outcome
                {
                    if (!scr.bust())
                    {
                        auto o = run_impl(ctx, s2, p2, d, burn_pile);
                        o *= double(prob);
                        chatter(ctx, "results in : ", o);
                        return outcome(o);
                    }
                    else
                    {
                        auto o = outcome(1, 0);
                        o *= prob;
                        return o;
                    }
                };
                outcomes.push_back(handle_score());
            }
            else
            {
                chatter(ctx, "no ", c, " in shoe");
            }
        }

        double invested = 0.0;
        double pay = 0.0;
        for (auto &&o : outcomes)
        {
            invested += o.invested * o.probability;
            pay += o.returned * o.probability;
        }

        return outcome(invested, pay);
    }

    auto
    hit_player_once(
        shoe s,
        player_hand const &p,
        dealer_hand const &d,
        cards burn_pile)
    -> outcome
    {
        if (s.exhausted())
            s += std::move(burn_pile);
        polyfill::static_vector<outcome, nof_card_scales> outcomes;
        int total_cards_in_shoe = 0;
        for (auto c : all_card_faces())
        {
            if (auto avail = s[c];avail)
            {
                total_cards_in_shoe += avail;

                auto s2 = s;
                auto p2 = p;
                deal_one(s2, p2, c);
                outcomes.push_back(dealers_turn(s2, score(p2), d, burn_pile) * double(avail));
            }
        }

        auto scale = 1.0 / double(total_cards_in_shoe);

        double invested = 0.0;
        double pay = 0.0;
        for (auto &&o : outcomes)
        {
            invested += o.invested * o.probability;
            pay += o.returned * o.probability;
        }
        invested *= scale;
        pay *= scale;

        return outcome(invested, pay);
    }

    struct context;

    inline auto run_impl(context const& ctx,
        shoe const &s,
                         player_hand const &p,
                         dealer_hand const &d,
                         cards const &burn_pile)
                         -> scenario_result
    {
        auto key = std::tie(p, d, s, burn_pile);
        auto imemo = chat_ ? player_memo_.end() : player_memo_.find(key);
        if (imemo == player_memo_.end())
        {
            auto possible_results = polyfill::static_vector<scenario_result, 4>();

            if (rules_.may_stick(p))
            {
                chatter(ctx, "consider stick:");
                auto &res =
                    possible_results.push_back(scenario_result(player_action::stick));
                auto o = dealers_turn(s, score(p), d, burn_pile);
                chatter(ctx, "would result in :", o);
                res.update(o);
            }

            if (rules_.may_hit(p))
            {
                chatter(ctx, "consider card:");
                auto &res =
                    possible_results.push_back(scenario_result(player_action::hit));
                auto o = hit_player(s, p, d, burn_pile);
                chatter(ctx, "would result in :", o);
                res.update(o);
            }
            if (rules_.may_double(p))
            {
                chatter(ctx, "consider double:");
                auto &res = possible_results.push_back(
                    scenario_result(player_action::double_down));
                auto o = hit_player_once(s, p, d, burn_pile);
                o.double_down();
                chatter(ctx, "would result in :", o);
                res.update(o);
            }
            /*
            if (r.may_split(p))
                possible_results.push_back(scenario_result(player_action::split));
        */

            imemo = player_memo_.emplace(key, best_of(possible_results)).first;
        }

        return imemo->second;
    }

    inline auto
    run(
        shoe const &s,
        player_hand const &p,
        dealer_hand const &d,
        cards const &burn_pile)
    -> scenario_result
    {
        auto ctx = recursing() ? context() : context(to_string(p));
        return run_impl(ctx, s, p, d, burn_pile);
    }

    auto
    deal_one(
        shoe &s,
        hand &d,
        card_scale cs) -> void
    {
        s -= cs;
        d += cs;
    }

    auto
    dealers_turn(
        shoe const &s,
        score const &player_score,
        dealer_hand const &d,
        cards const &burn_pile) -> outcome
    {
        auto key = std::tie(player_score, d, s, burn_pile);
        auto imemo = memo_.find(key);
        if (imemo != memo_.end())
        {
            return imemo->second;
        }

        auto accumulated_deal_one = [&] {
            polyfill::static_vector<outcome, nof_card_scales> outcomes;
            int total_cards_in_shoe = 0;
            for (std::size_t i = 0; i < nof_card_scales; ++i)
            {
                auto avail = s[to_card_scale(i)];
                if (avail)
                {
                    total_cards_in_shoe += avail;

                    auto bp2 = burn_pile;
                    auto s2 = s;
                    if (s2.exhausted())
                        s2 += std::move(bp2);
                    auto d2 = d;
                    deal_one(s2, d2, to_card_scale(i));
                    outcomes.push_back(dealers_turn(s2, player_score, d2, bp2) *
                                       double(avail));
                }
            }

            auto scale = 1.0 / double(total_cards_in_shoe);

            double invested = 0.0;
            double pay = 0.0;
            for (auto &&o : outcomes)
            {
                invested += o.invested * o.probability;
                pay += o.returned * o.probability;
            }
            invested *= scale;
            pay *= scale;

            auto result = outcome(invested, pay);
            memo_.emplace(key, result);
            return result;
        };

        auto dealer_score = score(d);
        switch (rules_.select_dealer_action(dealer_score))
        {
        case dealer_action::hit:
        {
            return accumulated_deal_one();
        }
        case dealer_action::stand:
        {
            return outcome(1, rules_.payoff(player_score, dealer_score));
        }
        }
        assert(!"logic error");
        return outcome();
    }


    void
    chat(std::ostream *logger)
    {
        chat_ = logger;
    }

    struct context
    {
        friend std::ostream& operator<<(std::ostream& os, context const& co)
        {
            os << context_string_;
            return os;
        }

        explicit context(char c = ' ')
            : adjust_(1)
        {
            context_string_ += c;
        }

        explicit context(std::string const& s)
            : adjust_(s.size())
        {
            context_string_ += s;
        }

        explicit context(const char* p)
            : adjust_(std::strlen(p))
        {
            context_string_ += p;
        }

        context(context&& other)
        : adjust_(std::exchange(other.adjust_, 0))
        {
        }

        ~context()
        {
            context_string_.erase(context_string_.end() - adjust_, context_string_.end());
        }

        int adjust_;
    };

    template<class...Args>
    void chatter(context const& ctx, Args&&...args) const
    {
        if (not chat_)
            return;

        auto& log = *chat_;
        log << ctx << ' ';

        ((log << args), ...);

        log << '\n';
    }

    bool recursing() const {
        return !context_string_.empty();
    }


    rules const &rules_;
    memo_map memo_;

    player_memo_map player_memo_;
    std::ostream *chat_ = nullptr;
    static thread_local std::string context_string_;
};

thread_local inline std::string scenario::context_string_ = "";

} // namespace blackjack