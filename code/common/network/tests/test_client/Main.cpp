// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

// stupid windows
#undef GetMessage
#undef GetMessageW

#include <thread>

#include <fmt/format.h>
#include <TCPClient.h>

#include "flatbuffers/flatbuffers.h"

#include "protocol/generated/Message_generated.h"
#include "protocol/generated/Handshake_generated.h"
#include "protocol/generated/IdaSync_generated.h"

#include "utility/Thread.h"

using namespace std::chrono_literals;

class TestClient2 final : public network::TCPClientConsumer {
public:
  explicit TestClient2() :
      _client(*this)
  {
	_run = _client.Connect(nullptr, network::constants::kServerPort);

	if (!_run)
		fmt::print("Oh no! connect() failed with '{}'\n", _client.LastError());
	else {
		fmt::print("Connection: OK!\n");

		Hello();
	}
  }

  ~TestClient2()
  {
	if(_run)
	  _client.Disconnect();
  }

  void Hello()
  {
	network::FbsBuffer buffer;

	auto request = protocol::CreateHandshakeRequestDirect(
	    buffer, 1337, "", "{1337-1337-1337}", "TestClient");

	_client.SendPacket(protocol::MsgType_HandshakeRequest, buffer, request.Union());
  }

  void ConsumeMessage(const uint8_t *data, size_t len) override
  {
	flatbuffers::Verifier verifier(data, len);
	if(!protocol::VerifyMessageBuffer(verifier))
	  return;

	const protocol::Message *message = protocol::GetMessage(static_cast<const void *>(data));
	if(message->msg_type() == protocol::MsgType_HandshakeAck)
	  return HandleAuth(message);
  }

  void HandleAuth(const protocol::Message* msg)
  {
	  auto* m = msg->msg_as_HandshakeAck();
	  fmt::print("Recieved handshake acknowledgement, active users: {}", m->numUsers());
  }

  void OnDisconnect(int reason) override
  {
  }

  bool ShouldRun() const
  {
	return _run;
  }

  void Tick()
  {
	_client.Tick();
  }

private:
  bool _run;
  network::TCPClient _client;
};

int main()
{
  network::ScopedSocket sockInit;

  fmt::print("Initializing test_client\n");
  TestClient2 client;

  while(client.ShouldRun()) {
	client.Tick();
	std::this_thread::sleep_for(1ms);
  }

  fmt::print("Exiting test_client\n");
  return 0;
}