// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>

#include <TCPServer.h>
#include <fmt/format.h>

// stupid windows
#undef GetMessage
#undef GetMessageW

#include "flatbuffers/flatbuffers.h"

#include "protocol/generated/Handshake_generated.h"
#include "protocol/generated/IdaSync_generated.h"
#include "protocol/generated/Message_generated.h"

#include "utility/Thread.h"

using namespace std::chrono_literals;

class TestServer2 final : public network::TCPServerConsumer {
 public:
  explicit TestServer2() : _server(*this) {
    _run = _server.Host(network::constants::kServerPort);
  }

  ~TestServer2() {}

  void ConsumeMessage(network::connectid_t id,
                      const uint8_t* ptr,
                      size_t size) override {
    fmt::print("Consume(): {}: {}\n", static_cast<const void*>(ptr), size);
  }

  void OnConnection(network::connectid_t id) override {
    fmt::print("OnConnection(): -> {}\n", id);
  }

  void OnDisconnection(network::connectid_t id) override {
    const auto* peer = _server.PeerById(id);
    if (!peer) {
      fmt::print("Network crime!!!!!!\n");
      return;
    }

    fmt::print("OnDisconnection(): -> {} {}\n", id, peer->addr.to_string());
  }

  bool ShouldRun() const { return _run; }

  void Tick() { _server.Tick(); }

 private:
  bool _run = true;
  network::TCPServer _server;
};

int main() {
  network::ScopedSocket sockInit;

  fmt::print("Initializing test_server\n");
  TestServer2 server;

  while (server.ShouldRun()) {
    server.Tick();
    std::this_thread::sleep_for(1ms);
  }

  fmt::print("Exiting test_server\n");
  return 0;
}