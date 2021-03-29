// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <fmt/format.h>
#include <network/tcp/tcp_client.h>
#include <network/base/context_holder.h>

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
  running_ = client_.Connect("localhost", 4434);

  if (!running_)
    fmt::print("Failed to connect!\n");
  else
    fmt::print("Connected to {}\n", client_.Address().to_string());
}

void TestClient::OnConnection(const sockpp::inet_address& address) {
  fmt::print("OnConnection() -> connected to {}\n", address.to_string());
}

void TestClient::OnDisconnected(QuitReason reason) {
  fmt::print("OnDisconnected() -> disconnected with reason {}\n",
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

    if (!running_)
      fmt::print("Run() -> update function requested quit\n");
  }

  fmt::print("Run() -> after run\n");
}

int main(int argc, char** argv) {
  ContextHolder netContext;
  fmt::print("Initializing test_client\n");

  TestClient client;
  client.Run();

  return 0;
}