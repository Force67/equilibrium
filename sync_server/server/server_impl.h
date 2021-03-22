// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Server.h"

#include "data_handler.h"
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
  // impl for: TCPServerConsumer
  void OnDisconnection(network::connectid_t) override;
  void ConsumeMessage(network::connectid_t,
                      const uint8_t* ptr,
                      size_t size) override;

  void HandleAuth(network::connectid_t, const protocol::MessageRoot*);

 private:
  bool _listening = false;
  std::string _loginToken = "";

  network::Context _context;
  network::TCPServer _server;
  UserRegistry _userRegistry;
  DataHandler _dataHandler;

  flatbuffers::FlatBufferBuilder _fbb;

  using timestamp_t = std::chrono::high_resolution_clock::time_point;
  timestamp_t _tickTime;
  float _freeTime = 0;
};
}  // namespace sync_server