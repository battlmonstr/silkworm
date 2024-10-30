/*
   Copyright 2024 The Silkworm Authors

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

#include <catch2/catch_test_macros.hpp>

#include <silkworm/db/datastore/mdbx/mdbx.hpp>
#include <silkworm/infra/common/directories.hpp>

#include "silkworm.h"

namespace silkworm {

static db::EnvConfig make_test_env_config(const TemporaryDirectory& tmp_dir) {
    return db::EnvConfig{
        .path = tmp_dir.path().string(),
        .create = true,
        .exclusive = true,
        .in_memory = true,
        .shared = false,
    };
}

TEST_CASE("mdbx::env::start_read") {
    TemporaryDirectory tmp_dir;
    db::EnvConfig env_config = make_test_env_config(tmp_dir);
    auto env = db::open_env(env_config);
    MDBX_env* env_ptr = env;
    db::EnvUnmanaged env2{env_ptr};
    REQUIRE_NOTHROW(env2.start_read());
}

TEST_CASE("mdbx::env::start_read_from_capi") {
    TemporaryDirectory tmp_dir;
    db::EnvConfig env_config = make_test_env_config(tmp_dir);
    auto env = db::open_env(env_config);
    MDBX_env* env_ptr = env;
    REQUIRE(silkworm_test_env_start_read(env_ptr) == SILKWORM_OK);
}

}  // namespace silkworm
