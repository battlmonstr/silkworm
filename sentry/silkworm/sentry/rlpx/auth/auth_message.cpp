/*
Copyright 2020-2022 The Silkworm Authors

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

#include "auth_message.hpp"
#include <algorithm>
#include <functional>
#include <silkworm/common/endian.hpp>
#include <silkworm/common/secp256k1_context.hpp>
#include <silkworm/rlp/encode_vector.hpp>
#include <silkworm/sentry/common/ecc_key_pair.hpp>
#include <silkworm/sentry/common/random.hpp>
#include "ecies_cipher.hpp"

namespace silkworm::sentry::rlpx::auth {

static Bytes sign(ByteView data, ByteView private_key) {
    SecP256K1Context ctx{/* allow_verify = */ false, /* allow_sign = */ true};
    secp256k1_ecdsa_signature signature;
    bool ok = ctx.sign(&signature, data, private_key);
    if (!ok) {
        throw std::runtime_error("Failed to sign an AuthMessage");
    }

    return ctx.serialize_signature(&signature);
}

AuthMessage::AuthMessage(common::EccPublicKey initiator_public_key, common::EccPublicKey recipient_public_key)
    : initiator_public_key_(std::move(initiator_public_key)),
      recipient_public_key_(std::move(recipient_public_key)) {

    common::EccKeyPair ephemeral_key_pair;
    Bytes shared_secret = EciesCipher::compute_shared_secret(recipient_public_key_, ephemeral_key_pair.private_key());

    nonce_ = common::random_bytes(shared_secret.size());

    // shared_secret ^= nonce_
    std::transform(shared_secret.cbegin(), shared_secret.cend(), nonce_.cbegin(), shared_secret.begin(), std::bit_xor<>{});
    signature_ = sign(shared_secret, ephemeral_key_pair.private_key());
}

AuthMessage::AuthMessage(ByteView)
    : initiator_public_key_({{}}),
      recipient_public_key_({{}}) {
    // TODO
}

Bytes AuthMessage::body_as_rlp() const {
    Bytes data;
    rlp::encode(data, signature_, initiator_public_key_.serialized(), nonce_, version);
    return data;
}

Bytes AuthMessage::body_encrypted() const {
    Bytes body = body_as_rlp();
    body.resize(EciesCipher::round_up_to_block_size(body.size()));
    return EciesCipher::encrypt(body, recipient_public_key_);
}

Bytes AuthMessage::serialize() const {
    Bytes body = body_encrypted();

    Bytes size(sizeof(uint16_t), 0);
    endian::store_big_u16(size.data(), static_cast<uint16_t>(body.size()));

    return size + body;
}

}  // namespace silkworm::sentry::rlpx::auth
