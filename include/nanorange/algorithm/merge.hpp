// nanorange/algorithm/merge.hpp
//
// Copyright (c) 2018 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NANORANGE_ALGORITHM_MERGE_HPP_INCLUDED
#define NANORANGE_ALGORITHM_MERGE_HPP_INCLUDED

#include <nanorange/algorithm/copy.hpp>
#include <nanorange/detail/algorithm/result_types.hpp>

NANO_BEGIN_NAMESPACE

template <typename I1, typename I2, typename O>
using merge_result = in_in_out_result<I1, I2, O>;

namespace detail {

struct merge_fn {
private:
    template <typename I1, typename S1, typename I2, typename S2, typename O,
              typename Comp, typename Proj1, typename Proj2>
    static constexpr merge_result<I1, I2, O>
    impl(I1 first1, S1 last1, I2 first2, S2 last2, O result, Comp& comp,
         Proj1& proj1, Proj2& proj2)
    {
        while (first1 != last1) {
            // If we've reached the end of the second range, copy any remaining
            // elements from the first range directly
            if (first2 == last2) {
                auto res = nano::copy(std::move(first1),
                                      std::move(last1),
                                      std::move(result));
                first1 = std::move(res.in);
                result = std::move(res.out);
                break;
            }

            // (Only) if the element from range2 compares less than the element
            // from range1, copy it. Otherwise copy the element from the first
            if (nano::invoke(comp, nano::invoke(proj2, *first2),
                             nano::invoke(proj1, *first1))) {
                *result = *first2;
                ++first2;
            } else {
                *result = *first1;
                ++first1;
            }
            ++result;
        }

        // We've reached the end of range1, so copy any remaining elements
        // from range2
        auto res = nano::copy(std::move(first2), std::move(last2),
                              std::move(result));
        first2 = std::move(res.in);
        result = std::move(res.out);

        return {std::move(first1), std::move(first2),  std::move(result)};
    }

public:
    template <typename I1, typename S1, typename I2, typename S2, typename O,
              typename Comp = ranges::less, typename Proj1 = identity,
              typename Proj2 = identity>
    constexpr std::enable_if_t<input_iterator<I1> && sentinel_for<S1, I1> &&
                                   input_iterator<I2> && sentinel_for<S2, I2> &&
        weakly_incrementable<O> &&
                                   mergeable<I1, I2, O, Comp, Proj1, Proj1>,
        merge_result<I1, I2, O>>
    operator()(I1 first1, S1 last1, I2 first2, S2 last2, O result,
               Comp comp = Comp{}, Proj1 proj1 = Proj1{}, Proj2 proj2 = Proj2{}) const
    {
        return merge_fn::impl(std::move(first1), std::move(last1),
                              std::move(first2), std::move(last2),
                              std::move(result), comp,
                              proj1, proj2);
    }

    template <typename Rng1, typename Rng2, typename O, typename Comp = ranges::less,
              typename Proj1 = identity, typename Proj2 = identity>
    constexpr std::enable_if_t<
        input_range<Rng1> && input_range<Rng2> &&
        weakly_incrementable<O> &&
            mergeable<iterator_t<Rng1>, iterator_t<Rng2>, O, Comp, Proj1, Proj2>,
        merge_result<borrowed_iterator_t<Rng1>, borrowed_iterator_t<Rng2>, O>>
    operator()(Rng1&& rng1, Rng2&& rng2, O result, Comp comp = Comp{},
               Proj1 proj1 = Proj1{}, Proj2 proj2 = Proj2{}) const
    {
        return merge_fn::impl(nano::begin(rng1), nano::end(rng1),
                              nano::begin(rng2), nano::end(rng2),
                              std::move(result), comp,
                              proj1, proj2);
    }
};

}

NANO_INLINE_VAR(detail::merge_fn, merge)

NANO_END_NAMESPACE

#endif
