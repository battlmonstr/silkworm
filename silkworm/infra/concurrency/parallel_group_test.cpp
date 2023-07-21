/*
   Copyright 2023 The Silkworm Authors

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

#include "parallel_group_utils.hpp"

#include <catch2/catch.hpp>

#include "../test_util/task_runner.hpp"

#include "awaitable_wait_for_one.hpp"
#include "timeout.hpp"
#include <boost/system/errc.hpp>
#include <boost/system/system_error.hpp>

namespace silkworm::concurrency {

Task<void> f() {
    using namespace awaitable_wait_for_one;
    using namespace std::chrono_literals;
    co_await (generate_parallel_group_task(100, [](size_t index) -> Task<void> {
        throw boost::system::system_error(make_error_code(boost::system::errc::operation_canceled));
        co_return;
        //co_await timeout(1s);
    }) || timeout(1ms));
}

TEST_CASE("parallel_group.co_spawn_cancellation_handler") {
    for (int i = 0; i < 1000; i++) {
        test_util::TaskRunner runner;
        CHECK_THROWS(runner.run(f()));
    }
}

}  //
