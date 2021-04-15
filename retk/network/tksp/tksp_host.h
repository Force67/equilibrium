// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/tksp/tksp_delegate.h>
#include <base/detached_queue.h>

namespace base {
template <typename T, size_t, size_t, size_t>
struct object_pool;
}

namespace network::tksp {
// yeet me!!
using namespace std::chrono_literals;

// base class for tksp server/client implementations
class Host {
 public:
  // distance in which between we select a port
  static constexpr uint16_t kPortRange = 10;
  // send keepalive
  static constexpr int kKeepAliveCount = 45;
  // ping rate
  static constexpr auto kPingRate = 500ms;

  explicit Host(TkspDelegate& del);
  ~Host();

  bool IsClient() const { return peer_list_.size() == 1; }

  bool DropPeer(PeerBase::Id);
  PeerBase* FindPeer(PeerBase::Id);

  void BroadcastPacket(const uint8_t* data,
                       size_t size,
                       PeerBase::Id exclude_id);

 protected:
  void ReadChunk(PeerBase& peer);
  void WriteChunks();

  void Process();

  void QueueOutgoingCommand(Chunkheader::Type command_type,
                            PeerBase::Id identifier,
                            const uint8_t* data,
                            size_t size);

  virtual void OnChunkEvent() = 0;
  virtual bool SetSocketOptions() = 0;

 private:
  uint8_t chunk_buffer_[Chunkheader::kMaxSize]{};

    struct OutgoingCommand;

  // temporary
  using CommandQueue =
      base::object_pool<OutgoingCommand, 1ull * 1024 * 1024, 5, 1>;

  // pooled thread-safe command list
  base::detached_mpsc_queue<OutgoingCommand> outgoing_queue_;
  std::unique_ptr<CommandQueue> pool_;

 protected:
  TkspDelegate& delegate_;

  // random number used for a number of things, including assigning unique
  // ids to peers.
  uint32_t seed_;

  // collection of peers
  std::vector<std::unique_ptr<PeerBase>> peer_list_;
};
}  // namespace network::tksp