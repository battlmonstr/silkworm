#[[
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
]]

set(MAX_LINE_LENGTH 120)

function(check file_path)
    math(EXPR OVER_LEN "${MAX_LINE_LENGTH} + 1")
    file(STRINGS "${file_path}" lines LENGTH_MINIMUM ${OVER_LEN})

    foreach(line IN LISTS lines)
        message(SEND_ERROR "${file_path}: line exceeds ${MAX_LINE_LENGTH} chars")
        message(${line})
    endforeach()
endfunction()

cmake_policy(SET CMP0009 NEW)
file(
    GLOB_RECURSE SRC
    LIST_DIRECTORIES false
    "cmd/*.?pp"
    "core/*.?pp"
    "node/*.?pp"
    "sentry/*.?pp"
    "wasm/*.?pp"
)

# exceptions
list(FILTER SRC EXCLUDE REGEX "core/silkworm/chain/intrinsic_gas_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "core/silkworm/trie/hash_builder\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "core/silkworm/types/transaction_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "cmd/test/backend_kv_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/downloader/packets/packet_coding_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/rpc/client/call\\.hpp\$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/rpc/client/call_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/rpc/client/sentry_calls\\..pp\$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/rpc/conversion_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/rpc/server/backend_calls\\..pp\$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/rpc/server/backend_kv_server_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/rpc/server/call\\.hpp\$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/rpc/server/kv_calls\\..pp\$")
list(FILTER SRC EXCLUDE REGEX "sentry/silkworm/sentry/common/ecc_key_pair_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "sentry/silkworm/sentry/common/enode_url_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "sentry/silkworm/sentry/rlpx/auth/ecies_cipher_test\\.cpp\$")
list(FILTER SRC EXCLUDE REGEX "sentry/silkworm/sentry/rpc/service\\.cpp\$")

foreach(F IN LISTS SRC)
    check("${F}")
endforeach()
