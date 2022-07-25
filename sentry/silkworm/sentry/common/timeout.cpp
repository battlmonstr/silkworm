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

#include "timeout.hpp"
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/this_coro.hpp>

namespace silkworm::sentry::common {

boost::asio::awaitable<void> Timeout::schedule() const {
    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::deadline_timer timer(executor);
    timer.expires_from_now(boost::posix_time::milliseconds(duration_.count()));
    co_await timer.async_wait(boost::asio::use_awaitable);
}

}  // namespace silkworm::sentry::common
