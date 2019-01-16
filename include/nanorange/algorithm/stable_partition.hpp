// nanorange/algorithm/stl/stable_partition.hpp
//
// Copyright (c) 2019 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//===-------------------------- algorithm ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NANORANGE_ALGORITHM_STABLE_PARTITION_HPP_INCLUDED
#define NANORANGE_ALGORITHM_STABLE_PARTITION_HPP_INCLUDED

#include <nanorange/algorithm/rotate.hpp>

NANO_BEGIN_NAMESPACE

namespace detail {

struct stable_partition_fn {
private:
    // Note to self: this is a closed range, last is NOT past-the-end!
    template <typename I, typename Pred, typename Proj>
    static I impl_unbuffered(I first, I last, iter_difference_t<I> dist,
                             Pred& pred, Proj& proj)
    {
        using dist_t = iter_difference_t<I>;

        if (dist == 2) {
            // We know first is false and last is true, so swap them
            nano::iter_swap(first, last);
            return last;
        }

        if (dist == 3) {
            // We know first is false and last-1 is true, so look at middle
            I middle = nano::next(first);

            if (nano::invoke(pred, nano::invoke(proj, *middle))) {
                nano::iter_swap(first, middle);
                nano::iter_swap(middle, last);
                return last;
            }

            // middle is false
            nano::iter_swap(middle, last);
            nano::iter_swap(first, middle);
            return middle;
        }

        const dist_t half = dist/2;
        const I middle = nano::next(first, half);

        I m1 = nano::prev(middle);
        dist_t len_half = half;

        while (m1 != first && !nano::invoke(pred, nano::invoke(proj, *m1))) {
            --len_half;
            --m1;
        }

        const I first_false = (m1 == first) ? first :
                impl_unbuffered(first, m1, len_half, pred, proj);

        m1 = middle;
        len_half = dist - half;

        while (nano::invoke(pred, nano::invoke(proj, *m1))) {
            if (++m1 == last) {
                return nano::rotate(first_false, middle, ++last).begin();
            }
        }

        const I last_false = impl_unbuffered(m1, last, len_half, pred, proj);

        return nano::rotate(first_false, middle, last_false).begin();
    }

    template <typename I, typename Pred, typename Proj>
    static I impl(I first, I last, Pred& pred, Proj& proj)
    {
        // Find the first non-true value
        while (true) {
            if (first == last) {
                return first;
            }
            if (!nano::invoke(pred, nano::invoke(proj, *first))) {
                break;
            }
            ++first;
        }

        // Find the last true value
        do {
            --last;
            if (last == first) {
                return last;
            }
            if (nano::invoke(pred, nano::invoke(proj, *last))) {
                break;
            }
        } while (true);

        const auto dist = nano::distance(first, last) + 1;
        // Note to self: [first, last] is a CLOSED range here!
        return impl_unbuffered(std::move(first), std::move(last), dist, pred, proj);
    }

    template <typename I, typename S, typename Pred, typename Proj>
    static std::enable_if_t<!Same<I, S>, I>
    impl(I first, S last, Pred& pred, Proj& proj)
    {
        return impl(first, nano::next(first, last), pred, proj);
    }

public:
    template <typename I, typename S, typename Pred, typename Proj = identity>
    std::enable_if_t<
        BidirectionalIterator<I> &&
        Sentinel<S, I> &&
        IndirectUnaryPredicate<Pred, projected<I, Proj>>, I>
    operator()(I first, S last, Pred pred, Proj proj = Proj{}) const
    {
        return stable_partition_fn::impl(std::move(first), std::move(last),
                                         pred, proj);
    }

    template <typename Rng, typename Pred, typename Proj = identity>
    std::enable_if_t<
        BidirectionalRange<Rng> &&
        IndirectUnaryPredicate<Pred, projected<iterator_t<Rng>, Proj>>,
    safe_iterator_t<Rng>>
    operator()(Rng&& rng, Pred pred, Proj proj = Proj{}) const
    {
        return stable_partition_fn::impl(nano::begin(rng), nano::end(rng),
                                         pred, proj);
    }
};

}

NANO_INLINE_VAR(detail::stable_partition_fn, stable_partition)

NANO_END_NAMESPACE

#endif
