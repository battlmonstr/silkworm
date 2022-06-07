/*
Copyright 2021-2022 The Silkworm Authors

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

#ifndef SILKWORM_BODY_SEQUENCE_HPP
#define SILKWORM_BODY_SEQUENCE_HPP

#include <list>

#include <silkworm/chain/identity.hpp>

#include <silkworm/downloader/packets/new_block_packet.hpp>
#include <silkworm/downloader/packets/block_bodies_packet.hpp>
#include <silkworm/downloader/packets/get_block_bodies_packet.hpp>

#include "db_tx.hpp"
#include "statistics.hpp"
#include "types.hpp"

namespace silkworm {

/** BodySequence represents the sequence of body that we are downloading.
 *  It has these responsibilities:
 *    - decide what bodies request (to peers)
 *    - collect bodies,
 *    - decide what bodies can be persisted on the db
 */
class BodySequence {
  public:
    BodySequence(const Db::ReadOnlyAccess&, const ChainIdentity&);
    ~BodySequence();

    // sync current state - this must be done at body forward
    void start_bodies_downloading(BlockNum highest_body_in_db, BlockNum highest_header_in_db);
    void stop_bodies_downloading();

    //! core functionalities: trigger the internal algorithms to decide what bodies we miss
    using MinBlock = BlockNum;
    auto request_more_bodies(time_point_t tp, uint64_t active_peers)
        -> std::tuple<GetBlockBodiesPacket66, std::vector<PeerPenalization>, MinBlock>;

    //! it needs to know if the request issued was not delivered
    void request_nack(time_point_t tp, const GetBlockBodiesPacket66&);

    //! core functionalities: process received bodies
    Penalty accept_requested_bodies(BlockBodiesPacket66&, const PeerId&);

    //! core functionalities: process received block announcement
    Penalty accept_new_block(const Block&, const PeerId&);

    //! core functionalities: returns bodies that are ready to be persisted
    auto withdraw_ready_bodies() -> std::vector<Block>;

    //! minor functionalities
    std::list<NewBlockPacket>& announces_to_do();

    [[nodiscard]] BlockNum highest_block_in_db() const;
    [[nodiscard]] BlockNum highest_block_in_memory() const;
    [[nodiscard]] BlockNum lowest_block_in_memory() const;
    [[nodiscard]] BlockNum target_height() const;
    [[nodiscard]] long outstanding_bodies(time_point_t tp) const;
    [[nodiscard]] bool has_bodies_to_request(time_point_t tp, uint64_t active_peers) const;

    [[nodiscard]] size_t deadlines() const;
    [[nodiscard]] const Download_Statistics& statistics() const;

    // downloading process tuning parameters
    static /*constexpr*/ seconds_t kRequestDeadline; // = std::chrono::seconds(30);
                                    // after this a response is considered lost it is related to Sentry's peerDeadline
    static /*constexpr*/ milliseconds_t kNoPeerDelay; // = std::chrono::milliseconds(500);
                                                                       // delay when no peer accepted the last request
    static /*constexpr*/ size_t kPerPeerMaxOutstandingRequests; // = 4;
    static /*constexpr*/ BlockNum kMaxBlocksPerMessage; // = 128;               // go-ethereum client acceptance limit
    static constexpr BlockNum kMaxAnnouncedBlocks = 10000;

  protected:
    void recover_initial_state();
    void make_new_requests(GetBlockBodiesPacket66&, MinBlock&, time_point_t tp, seconds_t timeout);
    auto renew_stale_requests(GetBlockBodiesPacket66&, MinBlock&, time_point_t tp, seconds_t timeout)
        -> std::vector<PeerPenalization>;
    void add_to_announcements(BlockHeader, BlockBody, Db::ReadOnlyAccess::Tx&);

    static bool is_valid_body(const BlockHeader&, const BlockBody&);

    struct BodyRequest {
        uint64_t request_id{0};
        Hash block_hash;
        BlockNum block_height{0};
        BlockHeader header;
        BlockBody body;
        time_point_t request_time;
        bool ready{false};
    };

    struct AnnouncedBlocks {
        void add(Block block);
        std::optional<BlockBody> remove(BlockNum bn);
        size_t size();
      private:
        std::map<BlockNum, Block> blocks_;
    };

    //using IncreasingHeightOrderedMap = std::map<BlockNum, BodyRequest>; // default ordering: less<BlockNum>
    struct IncreasingHeightOrderedRequestContainer: public std::map<BlockNum, BodyRequest> {
        using Impl = std::map<BlockNum, BodyRequest>;
        using Iter = Impl::iterator;

        std::list<Iter> find_by_request_id(uint64_t request_id);
        Iter find_by_hash(Hash oh, Hash tr);

        [[nodiscard]] BlockNum lowest_block() const;
        [[nodiscard]] BlockNum highest_block() const;
    };

    struct Deadlines {
        using Cardinality = size_t;

        void add(time_point_t tp, Cardinality cardinality) {
            using secs = std::chrono::seconds;
            auto rounded_tp = std::chrono::round<secs>(tp);
            auto d = deadlines_.find(rounded_tp);
            if (d == deadlines_.end())
                deadlines_.emplace(rounded_tp, cardinality);  // todo: use hints
            else
                d->second += cardinality;
        }

        void remove(time_point_t tp, Cardinality cardinality) {
            using secs = std::chrono::seconds;
            auto rounded_tp = std::chrono::round<secs>(tp);
            auto d = deadlines_.find(rounded_tp);
            if (d == deadlines_.end()) return;
            assert(d->second >= cardinality);
            d->second -= cardinality;
            if (d->second == 0) deadlines_.erase(d);
        }

        [[nodiscard]] Cardinality expired(time_point_t tp) const {
            using secs = std::chrono::seconds;
            auto rounded_tp = std::chrono::round<secs>(tp);
            Cardinality expired{0};
            auto d = deadlines_.begin();
            while(d != deadlines_.end() && d->first < rounded_tp) {  // todo: replace with accumulate + lower/upper_bound
                expired += d->second;
                d++;
            }
            return expired;
        }

        size_t size() const {
            return deadlines_.size();
        }

        void clear() {
            deadlines_.clear();
        }

      private:
        std::map<time_point_t,Cardinality> deadlines_; // ordered according to increasing times
    };

    IncreasingHeightOrderedRequestContainer body_requests_;
    AnnouncedBlocks announced_blocks_;
    std::list<NewBlockPacket> announcements_to_do_;

    Db::ReadOnlyAccess db_access_;
    [[maybe_unused]] const ChainIdentity& chain_identity_;

    bool in_downloading_{false};
    size_t ready_bodies_{0};
    BlockNum highest_body_in_db_{0};
    BlockNum headers_stage_height_{0};
    time_point_t last_nack_;
    Deadlines request_deadlines_;

    Download_Statistics statistics_;
};

}


#endif  // SILKWORM_BODY_SEQUENCE_HPP
