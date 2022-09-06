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

find_program(CLANG_TIDY clang-tidy)

set(CMAKE_BINARY_DIR "build" CACHE PATH "")

cmake_policy(SET CMP0009 NEW)
file(
    GLOB_RECURSE SRC
    LIST_DIRECTORIES false
    "cmd/check_pow.cpp"
    # "cmd/*.?pp"
    # "core/*.?pp"
    # "node/*.?pp"
    # "sentry/*.?pp"
    # "wasm/*.?pp"
)
list(FILTER SRC EXCLUDE REGEX "core/silkworm/chain/genesis_[a-z]+.cpp\$")
list(FILTER SRC EXCLUDE REGEX "core/silkworm/chain/dao.hpp$")
list(FILTER SRC EXCLUDE REGEX "node/silkworm/downloader/internals/preverified_hashes_[a-z]+.cpp\$")

# cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
execute_process(COMMAND "${CLANG_TIDY}" "-p=${CMAKE_BINARY_DIR}" ${SRC})
#execute_process(COMMAND "${CLANG_TIDY}" --enable-check-profile "-p=${CMAKE_BINARY_DIR}" ${SRC})
