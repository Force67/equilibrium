// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>

#include <TCPClient.h>
#include <fmt/format.h>

#include "flatbuffers/flatbuffers.h"
#include "protocol/generated/MessageRoot_generated.h"

#include "utility/Thread.h"

using namespace std::chrono_literals;

class TestClient2 final : public network::NetworkedClientComponent {
 public:
  TestClient2();
  ~TestClient2();

  void Logon();

  void ConsumeMessage(const uint8_t* data, size_t len) override {
    flatbuffers::Verifier verifier(data, len);
    if (!protocol::VerifyMessageRootBuffer(verifier))
      return;

    const protocol::MessageRoot* root =
        protocol::GetMessageRoot(static_cast<const void*>(data));

    fmt::print("ConsumeMessage() -> {}\n",
               protocol::EnumNameMsgType(root->msg_type()));

    switch (root->msg_type()) {
      case protocol::MsgType_HandshakeAck:
        return HandleAuth(root);
      case protocol::MsgType_UserEvent:
        return HandleUserEvent(root);
    }
  }

  void HandleUserEvent(const protocol::MessageRoot* root) {
    auto* msg = root->msg_as_UserEvent();
    fmt::print("UserEvent() -> Type: {} User: {}\n",
               protocol::EnumNameUserEventType(msg->type()),
               msg->userName()->str());
  }

  void HandleAuth(const protocol::MessageRoot* root) {
    auto* msg = root->msg_as_HandshakeAck();
    fmt::print("Recieved handshake ack, active users: {}\n", msg->numUsers());
  }

  void OnDisconnected(int reason) override {}

  bool ShouldRun() const { return _run; }

  void Tick() { _run = _client.Update(); }

 private:
  bool _run;
  network::TCPClient _client;
};

TestClient2::TestClient2() {
  _client.RegisterComponent(this);

  _run = _client.Connect(nullptr, network::kDefaultServerPort);

  if (!_run)
    fmt::print("Oh no! connect() failed with '{}'\n", _client.LastError());
  else {
    fmt::print("Connection: OK!\n");

    Logon();
  }
}

TestClient2::~TestClient2() {
  if (_run)
    _client.Disconnect();
}

void TestClient2::Logon() {
  network::FbsBuffer buffer;

  auto request = protocol::CreateHandshakeRequestDirect(
      buffer, 1337, "", "{1337-1337-1337}", "TestClient");

  _client.SendPacket(protocol::MsgType_HandshakeRequest, buffer,
                     request.Union());
}

int main() {
  network::Context netCtx;

  fmt::print("Initializing test_client\n");
  TestClient2 client;

  while (client.ShouldRun()) {
    client.Tick();
    std::this_thread::sleep_for(1ms);
  }

  fmt::print("Exiting test_client\n");
  return 0;
}