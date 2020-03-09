#pragma once
#include <iomanip>

namespace blackjack {

struct outcome {
  outcome(double invested = 1, double returned = 0)
      : invested(invested), returned(returned) {}

  void update(outcome const &other) noexcept { *this = other; }

  outcome &operator*=(double prob) {
    probability *= prob;
    return *this;
  }

  outcome &operator+=(outcome const& b) {
    assert(1.0 - probability < 0.999);
    invested += b.invested * b.probability;
    returned += b.returned * b.probability;
    return *this;
  }

  double payoff() const { return (returned - invested) / invested; }

  double invested;
  double returned;

  double probability = 1.0;
};

inline outcome operator*(outcome l, double prob) {
  l *= prob;
  return l;
}

inline std::ostream& operator<<(std::ostream& os, outcome const& o)
{
  os << "invested=" << o.invested << ", returned=" << o.returned << ", payoff=" << std::setprecision(4) << ((1.0+o.payoff()) * 100) << "%, probability=" << o.probability;
  return os;
}

} // namespace blackjack