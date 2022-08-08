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

#include "auth_ack_message.hpp"
#include <silkworm/rlp/decode.hpp>
#include <silkworm/rlp/encode_vector.hpp>
#include <silkworm/sentry/common/random.hpp>
#include "ecies_cipher.hpp"

namespace silkworm::sentry::rlpx::auth {

AuthAckMessage::AuthAckMessage(
    common::EccPublicKey initiator_public_key,
    common::EccPublicKey ephemeral_public_key)
    : initiator_public_key_(std::move(initiator_public_key)),
      ephemeral_public_key_(std::move(ephemeral_public_key)),
      nonce_(common::random_bytes(32)) {
}

AuthAckMessage::AuthAckMessage(
    ByteView data,
    const common::EccKeyPair& initiator_key_pair)
    : initiator_public_key_(initiator_key_pair.public_key()),
      ephemeral_public_key_(Bytes{}) {
    init_from_rlp(AuthAckMessage::decrypt_body(data, initiator_key_pair.private_key()));
}

Bytes AuthAckMessage::body_as_rlp() const {
    Bytes data;
    rlp::encode(data, ephemeral_public_key_.serialized(), nonce_, version);
    return data;
}

void AuthAckMessage::init_from_rlp(ByteView data) {
    Bytes public_key_data;
    auto err = rlp::decode(data, public_key_data, nonce_);
    if (err != DecodingResult::kOk) {
        throw std::runtime_error("Failed to decode AuthAckMessage RLP");
    }
    ephemeral_public_key_ = common::EccPublicKey::deserialize(public_key_data);
}

Bytes AuthAckMessage::body_encrypted() const {
    Bytes body = body_as_rlp();
    body.resize(EciesCipher::round_up_to_block_size(body.size()));
    return EciesCipher::encrypt(body, initiator_public_key_);
}

Bytes AuthAckMessage::decrypt_body(ByteView data, ByteView initiator_private_key) {
    return EciesCipher::decrypt(data, initiator_private_key);
}

Bytes AuthAckMessage::serialize() const {
    Bytes body = body_encrypted();

    Bytes size(sizeof(uint16_t), 0);
    endian::store_big_u16(size.data(), static_cast<uint16_t>(body.size()));

    return size + body;
}

}  // namespace silkworm::sentry::rlpx::auth
