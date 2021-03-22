// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Server.h"
#include "db_service.h"
#include "user_registry.h"

#include <sync/engine/sync_server.h>

namespace protocol {
struct MessageRoot;
}

namespace sync_server {

class ServerImpl final : public sync::SyncServerDelegate {
  friend class sync_server::Server;

 public:
  ServerImpl(int16_t port);
  ~ServerImpl() = default;

  Server::ResultStatus Initialize(bool useStorage);

  void Update();

  UserRegistry& Registry() { return _userRegistry; }

 private:
  // impl for: SyncServerDelegate
  void OnDisconnection(network::connectid_t) override;
  void ConsumeMessage(sync::cid_t, const protocol::MessageRoot*) override;
  void HandleAuth(network::connectid_t, const protocol::MessageRoot*);

 private:
  bool running_ = false;
  std::string _loginToken = "";

  network::Context netContext_;
  sync::SyncServer server_;
  UserRegistry _userRegistry;
  DbService dbService_;

  flatbuffers::FlatBufferBuilder fbb_;

  using timestamp_t = std::chrono::high_resolution_clock::time_point;
  timestamp_t timestamp_;
  float yieldTime_ = 0;
};
}  // namespace sync_server