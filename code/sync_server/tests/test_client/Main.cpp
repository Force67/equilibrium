// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>
#include <netlib/NetLib.h>
#include <netlib/ClientBase.h>

#include "flatbuffers/flatbuffers.h"
#include "moc_protocol/Handshake_generated.h"

using namespace std::chrono_literals;

class TestClient final : public netlib::NetClientBase {
public:
  TestClient()
  {
	NetClientBase::Connect(
		netlib::constants::kServerIp, 
		netlib::constants::kServerPort);
  }

  void OnConnection()
  {
	auto request = protocol::CreateHandshakeRequestDirect(
		_fbb, 1337, "", "{1337-1337-1337}", "TestClient", "");
  }

  void OnDisconnected(int status)
  {

  }

  void OnConsume(const uint8_t *data, size_t length)
  {

  }

private:
  flatbuffers::FlatBufferBuilder _fbb;
};

int main() {
  netlib::ScopedNetContext context;
  TestClient client;

  while (true) {
	client.Tick();
	std::this_thread::sleep_for(1ms);
  }

  return 0;
}