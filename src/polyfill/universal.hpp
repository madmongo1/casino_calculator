#pragma once
#include <boost/functional/hash.hpp>

namespace polyfill {

struct universal_hash {
  template <class T> std::size_t operator()(T &&x) const {
    auto op = boost::hash<std::decay_t<T>>();
    return op(x);
  }
};

using universal_equal_to = std::equal_to<void>;
} // namespace polyfill