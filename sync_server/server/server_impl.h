// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "server.h"
#include "user_registry.h"
#include "data_processor.h"

#include <network/base/context_holder.h>
#include <sync/engine/sync_server.h>

namespace protocol {
struct MessageRoot;
}

namespace sync_server {

class ServerImpl final : public network::ServerDelegate {
  friend class sync_server::Server;

 public:
  ServerImpl(int16_t port);
  ~ServerImpl() = default;

  Server::ResultStatus Initialize(bool withStorage);

  void Update();

  UserRegistry& Registry() { return users_; }

 private:
	 // impl: ServerDelegate
  void OnConnection(network::PeerId) override;
  void OnDisconnection(network::PeerId, network::QuitReason) override;
  void ProcessData(network::PeerId cid,
                   const uint8_t* data,
                   size_t len) override;

  void HandleAuth(network::PeerId, const protocol::MessageRoot*);

 private:
  bool running_ = false;
  std::string _loginToken = "";

  network::ContextHolder netContext_;
  sync::SyncServer server_;
  UserRegistry users_;
  std::unique_ptr<DataProcessor> dbService_;
  flatbuffers::FlatBufferBuilder fbb_;

  using timestamp_t = std::chrono::high_resolution_clock::time_point;
  timestamp_t timestamp_;
  float yieldTime_ = 0;
};
}  // namespace sync_server