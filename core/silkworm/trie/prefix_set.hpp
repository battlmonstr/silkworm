/*
   Copyright 2021-2022 The Silkworm Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef SILKWORM_TRIE_PREFIX_SET_HPP_
#define SILKWORM_TRIE_PREFIX_SET_HPP_

#include <vector>

#include <silkworm/common/base.hpp>

namespace silkworm::trie {

//! \brief A set of "nibbled" byte strings with the following property:
/// If x ∈ S and x starts with y, then y ∈ S.
/// Corresponds to RetainList in Erigon.
class PrefixSet {

  public:
    //! \brief Constructs an empty set.
    PrefixSet() = default;

    // copyable
    PrefixSet(const PrefixSet& other) = default;
    PrefixSet& operator=(const PrefixSet& other) = default;

    void insert(ByteView key);
    void insert(Bytes&& key);

    //! \brief Returns whether or not provided prefix is contained in any of the owned keys
    //! \remarks Doesn't change the set logically, but is not marked const since it's not safe to call this method concurrently.
    //! \see Erigon's RetainList::Retain
    bool contains(ByteView prefix);

  private:

    void ensure_sorted();

    std::vector<Bytes> nibbled_keys_;
    bool sorted_{false};
    size_t lte_index_{0}; // Index of the "LTE (<=)" key in the keys slice. Next one is "GT (>)"
    size_t max_index_{0}; // Maximum settable index (to avoid recompute vector.size();
};

}  // namespace silkworm::trie

#endif  // SILKWORM_TRIE_PREFIX_SET_HPP_
