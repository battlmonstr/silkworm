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

#include "find_neighbors.hpp"

#include <chrono>
#include <map>
#include <stdexcept>
#include <variant>

#include <boost/asio/this_coro.hpp>
#include <boost/system/errc.hpp>
#include <boost/system/system_error.hpp>

#include <silkworm/infra/common/log.hpp>
#include <silkworm/infra/concurrency/awaitable_wait_for_one.hpp>
#include <silkworm/infra/concurrency/channel.hpp>
#include <silkworm/infra/concurrency/timeout.hpp>
#include <silkworm/sentry/common/sleep.hpp>
#include <silkworm/sentry/discovery/disc_v4/common/ip_classify.hpp>
#include <silkworm/sentry/discovery/disc_v4/common/message_expiration.hpp>
#include <silkworm/sentry/discovery/disc_v4/common/node_address.hpp>
#include <silkworm/sentry/discovery/disc_v4/common/node_distance.hpp>

namespace silkworm::sentry::discovery::disc_v4::find {

NodeAddress na(const char* ip) {
    return NodeAddress{
        boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(ip), 30666),
        30666,
    };
}

Task<void> sleep_and_reply(concurrency::Channel<std::map<EccPublicKey, NodeAddress>>& ch) {
    using namespace std::chrono_literals;
    co_await sleep(300ms);

    ch.try_send({});
    co_await sleep(100ms);
    //    ch.try_send({});

    co_await sleep(100s);
}

Task<size_t> find_neighbors(
    EccPublicKey node_id,
    std::optional<boost::asio::ip::udp::endpoint> endpoint_opt,
    EccPublicKey local_node_id,
    MessageSender& message_sender,
    boost::signals2::signal<void(NeighborsMessage, EccPublicKey)>& on_neighbors_signal,
    node_db::NodeDb& db) {
    using namespace std::chrono_literals;
    using namespace concurrency::awaitable_wait_for_one;

    log::Debug("disc_v4") << "find_neighbors";

    boost::asio::ip::udp::endpoint endpoint = *endpoint_opt;

    auto executor = co_await boost::asio::this_coro::executor;
    concurrency::Channel<std::map<EccPublicKey, NodeAddress>> neighbors_channel{executor, 2};

    FindNodeMessage find_node_message{
        local_node_id,
        make_message_expiration(),
    };

    try {
        co_await message_sender.send_find_node(std::move(find_node_message), endpoint);
    } catch (const boost::system::system_error& ex) {
        if (ex.code() == boost::system::errc::operation_canceled)
            throw;
        log::Debug("disc_v4") << "find_neighbors failed to send_find_node"
                              << " to " << endpoint
                              << " due to exception: " << ex.what();
        co_return 0;
    }

    std::map<EccPublicKey, NodeAddress> neighbors_node_addresses;
    try {
        neighbors_node_addresses = std::get<0>(co_await (neighbors_channel.receive() || concurrency::timeout(500ms) || sleep_and_reply(neighbors_channel)));
    } catch (const concurrency::TimeoutExpiredError&) {
        co_return 0;
    }

    co_return neighbors_node_addresses.size();
}

}  // namespace silkworm::sentry::discovery::disc_v4::find
