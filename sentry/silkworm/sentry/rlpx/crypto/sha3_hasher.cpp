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
#include <stdexcept>
#include <openssl/evp.h>

namespace silkworm::sentry::rlpx::crypto {

Sha3Hasher::Sha3Hasher() : context_(EVP_MD_CTX_new()) {
    int ok = EVP_DigestInit(context_, EVP_sha3_256());
    if (!ok)
        throw std::runtime_error("Failed to init SHA3 hasher");
}

Sha3Hasher::~Sha3Hasher() {
    EVP_MD_CTX_free(context_);
}

void Sha3Hasher::update(ByteView data) {
    int ok = EVP_DigestUpdate(context_, data.data(), data.size());
    if (!ok)
        throw std::runtime_error("Failed to update a SHA3 hasher state");
}

Bytes Sha3Hasher::hash() {
    Bytes result(32, 0);
    int ok = EVP_DigestFinal_ex(context_, result.data(), nullptr);
    if (!ok)
        throw std::runtime_error("Failed to generate a SHA3 hash");

    ok = EVP_DigestInit_ex(context_, EVP_sha3_256(), nullptr);
    if (!ok)
        throw std::runtime_error("Failed to re-init SHA3 hasher");

    return result;
}

}  // namespace silkworm::sentry::rlpx::crypto
