// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <fmt/format.h>
#include <network/base/context_holder.h>
#include <network/tcp/tcp_server.h>

using namespace network;

class TestServer final : public ServerDelegate {
 public:
  TestServer();

  void OnConnection(connectid_t) override;
  void OnDisconnection(connectid_t) override;
  void ProcessData(connectid_t cid, const uint8_t* data, size_t len) override;

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

void TestServer::OnConnection(connectid_t connectId) {
  fmt::print("OnConnection() -> connectId ({})\n", connectId);
}

void TestServer::OnDisconnection(connectid_t connectId) {
  if (auto* peer = server_.PeerById(connectId)) {
    fmt::print("OnDisconnection() -> connectid ({}) address: {}\n", connectId,
               peer->Address().to_string());
  } else {
    fmt::print("Error!!!!");
  }
}

void TestServer::ProcessData(connectid_t connectId,
                             const uint8_t* data,
                             size_t len) {
  fmt::print("ProcessData() -> connectId ({}), length: {}\n", connectId, len);

  auto* header = reinterpret_cast<const Chunkheader*>(data);
  fmt::print("Message id: {}\n", static_cast<int>(header->id));
}

void TestServer::Run() {
  fmt::print("Run() -> Before run\n");

  while (running_) {
    server_.Update();
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