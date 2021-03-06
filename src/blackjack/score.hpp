#pragma once

#include "cards.hpp"
#include <ostream>

namespace blackjack {

struct score
{
    score(cards const &cards_) : value_(0)
        , soft_(false)
        , blackjack_(false)
    {
      if (cards_.count() == 2 and cards_[card_scale::ace] == 1 and
          cards_[card_scale::ten] == 1)
      {
        value_ = 21;
        blackjack_ = true;
        return;
      }

      auto current_value = 2;
      for (auto &&ncards : cards_)
      {
        value_ += current_value++ * ncards;
      }
      if (auto ace_count = cards_[card_scale::ace])
      {
        soft_ = true;
        value_ -= 10 * (ace_count - 1);
      }
    }

    int
    value() const
    { return (value_ > 21 && soft_) ? value_ - 10 : value_; }

    bool
    bust() const
    { return value() > 21; }

    bool
    soft() const
    { return soft_ and value_ <= 21; }

    bool
    blackjack() const
    { return blackjack_; }

    auto
    as_tuple() const
    { return std::tie(value_, soft_, blackjack_); }

    bool
    operator==(const score &r) const
    { return as_tuple() == r.as_tuple(); }

    friend std::size_t
    hash_value(score const &s)
    {
      using tuple_type = std::decay_t<decltype(s.as_tuple())>;
      auto op = boost::hash<tuple_type>();
      return op(s.as_tuple());
    }

    friend std::ostream& operator<<(std::ostream& os, score const& s)
    {
        if (s.bust())
            return os << "bust";
        if (s.blackjack())
            return os << "blackjack";
        if (s.soft())
            os << "soft ";
        return os << s.value();
    }

private:
    int value_;
    bool soft_;
    bool blackjack_;
};

} // namespace blackjack