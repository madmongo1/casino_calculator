#pragma once

#include <boost/functional/hash.hpp>
#include <memory>

namespace polyfill {

template <class T, std::size_t Capacity> struct static_vector {
  using value_type = T;
  using iterator = T *;
  using const_iterator = T const *;

  static_vector() : store_{}, size_(0) {}

  static constexpr std::size_t capacity() { return Capacity; }

  template <class U = T,
            std::enable_if_t<std::is_copy_constructible_v<U>> * = nullptr>
  static_vector(static_vector const &other) : static_vector() {
    try {
      for (auto &&x : other)
        push_back(x);
    } catch (...) {
      clear();
      throw;
    }
  }

  template <class U = T,
            std::enable_if_t<std::is_copy_constructible_v<U>> * = nullptr>
  static_vector &operator=(static_vector const &other) {
    auto tmp = other;
    *this = std::move(other);
  }

  template <class U = T,
            std::enable_if_t<std::is_move_constructible_v<U>> * = nullptr>
  static_vector(static_vector &&other) noexcept {
    for (auto &&x : other)
      push_back(std::move(x));
  }

  template <class U = T,
            std::enable_if_t<std::is_move_constructible_v<U>> * = nullptr>
  static_vector &operator=(static_vector &&other) noexcept {
    clear();
    for (auto &&x : other)
      push_back(std::move(x));
  }

  ~static_vector() noexcept { clear(); }

  const_iterator begin() const { return get(); }
  const_iterator end() const { return get() + size(); }

  T &operator[](std::size_t i) { return *(get() + i); }

  T const &operator[](std::size_t i) const { return *(get() + i); }

  T &push_back(T &&arg) {
    if (size() >= capacity())
      throw std::length_error("push_back");
    auto *p = get() + size_;
    p = new (p) T(std::move(arg));
    ++size_;
    return *p;
  }

  T &push_back(T const &arg) {
    if (size() >= capacity())
      throw std::length_error("push_back");
    auto *p = get(size_);
    p = new (p) T(arg);
    ++size_;
    return *p;
  }

  void pop_back() noexcept {
    auto p = get() + (--size_);
    p->~T();
  }

  std::size_t size() const { return size_; }

  bool empty() const { return size_ == 0; }

  void clear() noexcept {
    while (!empty())
      pop_back();
  }

private:
  auto get() const -> T const * {
    return std::addressof(reinterpret_cast<T const &>(store_));
  };

  auto get() -> T * { return std::addressof(reinterpret_cast<T &>(store_)); };

  std::aligned_storage_t<sizeof(T) * Capacity, alignof(T)> store_;
  std::size_t size_;
};

template <class T, std::size_t N>
auto hash_value(static_vector<T, N> const &v) -> std::size_t {
  auto seed = std::size_t(0);

  for (auto &&x : v)
    boost::hash_combine(seed, x);

  return seed;
}
} // namespace polyfill