#pragma once

#include "score.hpp"
#include "player_hand.hpp"
#include "dealer_hand.hpp"
#include <cassert>
#include <limits>

namespace blackjack {

enum class dealer_action
{
    stand,
    hit
};

inline auto
operator<<(
    std::ostream &os,
    dealer_action r) -> std::ostream &
{
    switch (r)
    {
    case dealer_action::hit: os << "dealer hits";
        break;
    case dealer_action::stand: os << "dealer stands";
        break;
    }
    return os;
}

enum class result
{
    player_win,
    player_blackjack,
    dealer_win,
    draw
};

inline auto
operator<<(
    std::ostream &os,
    result r) -> std::ostream &
{
    switch (r)
    {
    case result::player_win: os << "player win";
        break;
    case result::player_blackjack: os << "blackjack";
        break;
    case result::dealer_win: os << "dealer win";
        break;
    case result::draw: os << "draw";
        break;
    }
    return os;
}

struct rules
{

    auto
    compute_result(
        score const &player_score,
        score const &dealer_score) const -> result
    {
        if (dealer_score.blackjack())
            if (player_score.blackjack())
                return result::draw;
            else
                return result::dealer_win;
        else if (player_score.blackjack())
            return result::player_blackjack;

        if (player_score.bust())
            return result::dealer_win;

        if (dealer_score.bust())
            return result::player_win;

        if (player_score.value() > dealer_score.value())
            return result::player_win;
        if (player_score.value() < dealer_score.value())
            return result::dealer_win;
        return result::draw;
    }
    auto
    compute_result(
        player_hand const &player,
        dealer_hand const &dealer) const -> result
    {
        return compute_result(score(player), score(dealer));
    }

    auto
    payoff(result res) const -> double
    {
        switch (res)
        {
        case result::player_blackjack:return 1.0 + 1.5;
        case result::player_win:return 1.0 + 1.0;
        case result::dealer_win:return 0.0;
        case result::draw:return 1.0;
        }
        assert(!"logic error");
        return std::numeric_limits<double>::signaling_NaN();
    }

    auto
    payoff(score const& player_score, score const& dealer_score) const -> double
    {
        return payoff(compute_result(player_score, dealer_score));
    }

    bool
    may_split(player_hand const &player) const
    {
        if (player.is_pair())
            return true;
        return false;
    }

    bool
    may_stick(player_hand const &) const
    {
        return true;
    }

    bool
    may_hit(player_hand const &player) const
    {
      auto s = score(player);
      return not s.blackjack() and not s.bust();
    }

    bool
    may_double(player_hand const &player) const
    {
        return may_hit(player);
    }

    auto select_dealer_action(score const& dealer_score) const -> dealer_action
    {
        if (dealer_score.value() > 16)
            return dealer_action::stand;
        else
            return dealer_action::hit;
    }

    auto select_dealer_action(dealer_hand const& hand) const -> dealer_action
    {
        return select_dealer_action(score(hand));
    }

};

}