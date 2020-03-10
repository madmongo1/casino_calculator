#pragma once

#include <array>
#include <boost/functional/hash.hpp>
#include <numeric>

namespace blackjack {
enum card_scale
    : std::uint8_t
{
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

inline char to_char(card_scale card)
{
    switch (card)
    {
    case card_scale::two:return '2';
    case card_scale::three:return '3';
    case card_scale::four:return '4';
    case card_scale::five:return '5';
    case card_scale::six:return '6';
    case card_scale::seven:return '7';
    case card_scale::eight:return'8';
    case card_scale::nine:return '9';
    case card_scale::ten:return 'T';
    case card_scale::ace:return 'A';
    }
    return '?';
}

inline auto
operator<<(
    std::ostream &os,
    card_scale cs) -> std::ostream &
{
    return os << to_char(cs);
}

constexpr inline auto
to_index(card_scale scale) -> std::size_t
{
    return static_cast<std::size_t>(scale);
}

constexpr inline auto
to_card_scale(std::ptrdiff_t scale) -> card_scale
{
    return static_cast<card_scale>(scale);
}

constexpr int nof_card_scales = 10;

struct all_card_faces
{
    struct iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using value_type = card_scale;
        using reference = card_scale &;
        using pointer = card_scale *;
        using difference_type = std::ptrdiff_t;

        iterator(int i = nof_card_scales) : i_(i)
        {}

        auto
        operator*() const -> value_type
        {
            return to_card_scale(i_);
        }

        iterator &
        operator++()
        {
            ++i_;
            return *this;
        }

        iterator
        operator++(int)
        {
            auto result = *this;
            ++i_;
            return result;
        }

        bool
        operator==(iterator const &other) const
        { return i_ == other.i_; }

        bool
        operator!=(iterator const &other) const
        { return not(*this == other); }

    private:
        int i_;
    };

    iterator
    begin() const
    { return iterator(0); }

    iterator
    end() const
    { return iterator(nof_card_scales); }

};

struct cards
{
    using store_type = std::array<int, nof_card_scales>;
    using const_iterator = store_type::const_iterator;
    using value_type = store_type::value_type;

    struct modifier
    {
        cards *cards_;
        card_scale which_;
    };

    constexpr cards(std::initializer_list<card_scale> list) : store_{}
    {
        for (auto c : list)
            adjust(c);
    }

    template<class... CardScale,
        std::enable_if_t<(std::is_same_v<CardScale, card_scale> && ...)> * =
        nullptr>
    constexpr
    cards(CardScale... cs) : store_{}
    {
        (adjust(cs), ...);
    }

    constexpr cards()
        : store_{}
        , count_(0)
    {}

    int
    count(card_scale scale) const
    { return store_[to_index(scale)]; }

    int
    count() const
    { return count_; }

    bool empty() const { return count() == 0; }

    int const &
    operator[](card_scale scale) const
    {
        return store_[to_index(scale)];
    }

    /*
    int &
    operator[](card_scale scale)
    { return store_[to_index(scale)]; }
*/
    auto
    begin() const -> const_iterator
    { return store_.begin(); }

    auto
    end() const -> const_iterator
    { return store_.end(); }

    void
    adjust(
        card_scale cs,
        int n = 1)
    {
        store_[to_index(cs)] += n;
        count_ += n;
    }

    cards &
    operator-=(card_scale c)
    {
        adjust(c, -1);
        return *this;
    }

    cards &
    operator+=(card_scale c)
    {
        adjust(c);
        return *this;
    }

    cards &
    operator+=(cards&& other)
    {
        for (auto card : all_card_faces())
        {
            auto cnt = other.count(card);
            adjust(card, cnt);
            other.adjust(card, -cnt);
        }
        return *this;
    }

    cards &
    operator+=(cards const& other)
    {
        for (auto card : all_card_faces())
            adjust(card, other.count(card));
        return *this;
    }

    void clear()
    {
        std::fill(store_.begin(), store_.end(), 0);
    }

private:
    friend auto
    operator<<(
        std::ostream &os,
        cards const &c) -> std::ostream &
    {
        auto sep = "";
        if (c.count() == 0)
        {
            os << "empty";
        }
        else
        {
            for (auto i = nof_card_scales ; i-- ; )
            {
                auto card = to_card_scale(i);
                auto cnt = c.count(card);
                if (cnt)
                {
                    os << sep << card << "x" << c.count(card);
                    sep = ", ";
                }
            }
        }
        return os;
    }

    friend std::size_t
    hash_value(cards const &c)
    {
        auto op = boost::hash<std::array<int, nof_card_scales>>();
        return op(c.store_);
    }

    friend bool
    operator==(
        cards const &a,
        cards const &b)
    {
        return a.store_ == b.store_;
    }

protected:
    std::array<int, nof_card_scales> store_;
    int count_ = 0;
};

struct draw_probability
{
    draw_probability(cards c) : cards_(std::move(c))
    {}

    double
    update(card_scale cs)
    {
        if (auto cc = cards_.count(cs))
        {
            cards_ -= cs;
            return double(cc) / double(cards_.count());
        }
        else
        {
            return 0;
        }
    }

    cards cards_;
};

} // namespace blackjack
