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

#include <catch2/catch.hpp>

#include "../test_util/task_runner.hpp"
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/co_spawn.hpp>

#include "awaitable_wait_for_all.hpp"

namespace silkworm::concurrency {

using namespace boost::asio;
using namespace std::chrono_literals;

Task<void> sleep(std::chrono::milliseconds duration) {
    auto executor = co_await this_coro::executor;
    deadline_timer timer(executor);
    timer.expires_from_now(boost::posix_time::milliseconds(duration.count()));
    co_await timer.async_wait(use_awaitable);
}

Task<void> noop() {
    co_return;
}

Task<void> throw_op() {
    co_await sleep(1ms);
    throw std::runtime_error("throw_op");
    co_return;
}

Task<void> spawn_throw_op(boost::asio::strand<boost::asio::any_io_executor>& strand) {
    co_await co_spawn(strand, throw_op(), use_awaitable);
}

Task<void> spawn_noop_loop(boost::asio::strand<boost::asio::any_io_executor>& strand) {
    while (true) {
        co_await co_spawn(strand, noop(), use_awaitable);
    }
}

Task<void> f() {
    using namespace concurrency::awaitable_wait_for_all;
    auto executor = co_await this_coro::executor;
    auto strand = make_strand(executor);

    co_await (sleep(1s) && spawn_throw_op(strand) && spawn_noop_loop(strand));
}

TEST_CASE("parallel_group.co_spawn_cancellation_handler_bug") {
    while (true) {
        test_util::TaskRunner runner;
        CHECK_THROWS(runner.run(f()));
    }
}

}  //
