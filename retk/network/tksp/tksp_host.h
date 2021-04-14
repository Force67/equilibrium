// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/tksp/tksp_delegate.h>
#include <base/detached_queue.h>

namespace base {
template <typename T>
struct object_pool;
}

namespace network::tksp {

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

  bool IsClient() const { return peer_list_.size() == 1; }

 protected:
  void ReadChunk(PeerBase& peer);
  void WriteChunks();

  void Process();

  void QueueOutgoingCommand(Chunkheader::Type command_type,
                            PeerBase::Id identifier,
                            const uint8_t* data,
                            size_t size);

  virtual void OnChunkEvent() = 0;

 private:
  uint8_t chunk_buffer_[Chunkheader::kMaxSize]{};

  // pooled thread-safe command list
  struct OutgoingCommand;
  base::detached_mpsc_queue<OutgoingCommand> outgoing_queue_;
  std::unique_ptr<base::object_pool<OutgoingCommand>> pool_;

 protected:
  TkspDelegate& delegate_;

  // random number used for a number of things, including assigning unique
  // ids to peers.
  uint32_t seed_;

  // collection of peers
  std::vector<std::unique_ptr<PeerBase>> peer_list_;
};
}  // namespace network::tksp