// Copyright 2020-2022 The Silkworm Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "ecc_public_key.hpp"
#include <silkworm/common/secp256k1_context.hpp>
#include <silkworm/common/util.hpp>

namespace silkworm::sentry::common {

Bytes EccPublicKey::serialized_std() const {
    auto& data = data_;
    secp256k1_pubkey public_key;
    memcpy(public_key.data, data.data(), sizeof(public_key.data));

    SecP256K1Context ctx;
    return ctx.serialize_public_key(&public_key, /* is_compressed = */ false);
}

Bytes EccPublicKey::serialized() const {
    Bytes data = serialized_std();
    std::copy(data.cbegin() + 1, data.cend(), data.begin());
    data.pop_back();
    return data;
}

std::string EccPublicKey::hex() const {
    return ::silkworm::to_hex(serialized());
}

EccPublicKey EccPublicKey::deserialize(ByteView serialized_data) {
    SecP256K1Context ctx;
    secp256k1_pubkey public_key;
    bool ok = ctx.parse_public_key(&public_key, serialized_data);
    if (!ok) {
        throw std::runtime_error("Failed to parse an ephemeral public key");
    }
    return EccPublicKey{Bytes{public_key.data, sizeof(public_key.data)}};
}

}  // namespace silkworm::sentry::common
