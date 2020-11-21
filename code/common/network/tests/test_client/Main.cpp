// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>

#include <fmt/format.h>
#include <TCPClient.h>

// stupid windows
#undef GetMessage
#undef GetMessageW

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
	else
		fmt::print("Connection: OK!\n");
  }

  ~TestClient2()
  {
	if(_run)
	  _client.Disconnect();
  }

  void ConsumeMessage(const uint8_t *data, size_t len) override
  {
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
	network::FbsBuffer buffer;

	auto request = protocol::CreateHandshakeRequestDirect(
	    buffer, 1337, "", "{1337-1337-1337}", "TestClient");

	_client.SendPacket(protocol::MsgType_HandshakeRequest, buffer, request.Union());

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