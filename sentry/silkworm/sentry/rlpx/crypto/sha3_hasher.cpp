/*
   Copyright 2022 The Silkworm Authors

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

#include "sha3_hasher.hpp"
#include <keccak.h>

namespace silkworm::sentry::rlpx::crypto {

Sha3Hasher::Sha3Hasher() : state_(25 * sizeof(uint64_t), 0) {
}

void Sha3Hasher::update(ByteView data) {
    keccak(reinterpret_cast<uint64_t*>(state_.data()), 256, data.data(), data.size());
}

ByteView Sha3Hasher::hash() {
    return ByteView{state_.data(), 32};
}

}  // namespace silkworm::sentry::rlpx::crypto
