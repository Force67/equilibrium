// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <fmt/format.h>
#include <network/tcp/tcp_client.h>
#include <network/base/network_context.h>

using namespace network;

class TestClient final : public ClientDelegate {
 public:
  TestClient();

  void OnConnection(const sockpp::inet_address&) override;
  void OnDisconnected(QuitReason) override;
  void ProcessData(const uint8_t* ptr, size_t len) override;

  void Run();

 private:
  TCPClient client_;
  bool running_;
};

TestClient::TestClient() : client_(*this) {
  running_ = client_.Connect("localhost", 1337);

  if (!running_) {
    fmt::print("Failed to connect!\n");
  }
}

void TestClient::OnConnection(const sockpp::inet_address& address) {
  fmt::print("OnConnection() -> connected to {}\n", address.to_string());
}

void TestClient::OnDisconnected(QuitReason reason) {
  fmt::print("OnDisconnected() -> disconnected with reason {}",
             static_cast<int>(reason));
}

void TestClient::ProcessData(const uint8_t* data, size_t len) {
  fmt::print("ProcessData() -> length: {}\n", len);

  auto* header = reinterpret_cast<const Chunkheader*>(data);
  fmt::print("Message id: {}\n", static_cast<int>(header->id));
}

void TestClient::Run() {
  fmt::print("Run() -> before run\n");

  while (running_) {
    running_ = client_.Update();
  }

  fmt::print("Run() -> after run\n");
}

int main(int argc, char** argv) {
  Context netContext;

  TestClient client;
  client.Run();

  return 0;
}