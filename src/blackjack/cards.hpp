#pragma once

#include <array>
#include <boost/functional/hash.hpp>
#include <numeric>

namespace blackjack {
enum card_scale : std::uint8_t {
  two,
  three,
  four,
  five,
  six,
  seven,
  eight,
  nine,
  ten,
  ace
};

inline auto operator<<(std::ostream &os, card_scale cs) -> std::ostream & {
  switch (cs) {
  case card_scale::two:
    os << '2';
    break;
  case card_scale::three:
    os << '3';
    break;
  case card_scale::four:
    os << '4';
    break;
  case card_scale::five:
    os << '5';
    break;
  case card_scale::six:
    os << '6';
    break;
  case card_scale::seven:
    os << '7';
    break;
  case card_scale::eight:
    os << '8';
    break;
  case card_scale::nine:
    os << '9';
    break;
  case card_scale::ten:
    os << 'T';
    break;
  case card_scale::ace:
    os << 'A';
    break;
  }
  return os;
}

constexpr inline auto to_index(card_scale scale) -> std::size_t {
  return static_cast<std::size_t>(scale);
}

constexpr inline auto to_card_scale(std::ptrdiff_t scale) -> card_scale {
  return static_cast<card_scale>(scale);
}

constexpr int nof_card_scales = 10;

struct cards {
  using store_type = std::array<int, nof_card_scales>;
  using const_iterator = store_type::const_iterator;
  using value_type = store_type::value_type;

  constexpr cards(std::initializer_list<card_scale> list) : store_{} {
    for (auto c : list)
      ++store_[to_index(c)];
  }

  template <class... CardScale,
            std::enable_if_t<(std::is_same_v<CardScale, card_scale> && ...)> * =
                nullptr>
  constexpr cards(CardScale... cs) : store_{} {
    (++store_[to_index(cs)], ...);
  }

  constexpr cards() : store_{} {}

  int count(card_scale scale) const { return store_[to_index(scale)]; }
  int count() const { return std::accumulate(store_.begin(), store_.end(), 0); }

  int const &operator[](card_scale scale) const {
    return store_[to_index(scale)];
  }

  int &operator[](card_scale scale) { return store_[to_index(scale)]; }

  auto begin() const -> const_iterator { return store_.begin(); }

  auto end() const -> const_iterator { return store_.end(); }

  void add(card_scale cs, std::size_t n = 1) { store_[to_index(cs)] += n; }

private:
  friend auto operator<<(std::ostream &os, cards const &c) -> std::ostream & {
    auto sep = "";
    auto first = c.store_.rbegin();
    auto last = c.store_.rend();
    while (first != last) {
      auto count = *first++;
      auto card = to_card_scale(std::distance(c.store_.begin(), first.base()));
      while (count--) {
        os << sep << card;
        sep = ", ";
      }
    }
    return os;
  }

  friend std::size_t hash_value(cards const &c) {
    auto op = boost::hash<std::array<int, nof_card_scales>>();
    return op(c.store_);
  }

  friend bool operator==(cards const &a, cards const &b) {
    return a.store_ == b.store_;
  }

protected:
  std::array<int, nof_card_scales> store_;
};

} // namespace blackjack
