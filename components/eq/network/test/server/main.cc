// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <fmt/format.h>
#include <network/base/context_holder.h>
#include <network/tcp/tcp_server.h>

using namespace network;

class TestServer final : public ServerDelegate {
 public:
  TestServer();

  void OnConnection(PeerId) override;
  void OnDisconnection(PeerId, QuitReason) override;
  void ProcessData(PeerId cid, const uint8_t* data, size_t len) override;

  void Run();

 private:
  TCPServer server_;
  bool running_;
};

TestServer::TestServer() : server_(*this) {
  auto port = server_.TryHost(4434);
  running_ = port != -1;

  if (running_) {
    fmt::print("Hosting server at port {}\n", port);
  }
}

void TestServer::OnConnection(PeerId connectId) {
  fmt::print("OnConnection() -> connectId ({})\n", connectId);
}

void TestServer::OnDisconnection(PeerId connectId, QuitReason exitCode) {
  if (auto* peer = server_.PeerById(connectId)) {
    fmt::print(
        "OnDisconnection() -> connectid ({}) address: {}, exitcode: {}\n",
        connectId, peer->Address().to_string(), static_cast<int>(exitCode));
  } else {
    fmt::print("Error!!!!");
  }
}

void TestServer::ProcessData(PeerId connectId,
                             const uint8_t* data,
                             size_t len) {
  fmt::print("ProcessData() -> connectId ({}), length: {}", connectId, len);

  auto* header = reinterpret_cast<const Chunkheader*>(data);

  switch (header->id) {
    case CommandId::kPing:
      fmt::print(" -> Got Ping after {} ms\n",
                 server_.PeerById(connectId)->GetPing().count());
      break;
    case CommandId::kData:
      fmt::print(" -> Got Data\n");
      break;
    default:
      fmt::print(" -> Got Message id: {}\n", static_cast<int>(header->id));
      break;
  }
}

void TestServer::Run() {
  fmt::print("Run() -> Before run\n");

  while (running_) {
    server_.Tick();
  }

  fmt::print("Run() -> Run completed\n");
}

int main(int argc, char**) {
  ContextHolder netContext;
  fmt::print("Initializing test_server\n");

  TestServer server;
  server.Run();

  return 0;
}