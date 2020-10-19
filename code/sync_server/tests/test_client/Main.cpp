// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>
#include <netlib/NetLib.h>
#include <netlib/Client.h>

#include "flatbuffers/flatbuffers.h"

#include "moc_protocol/Message_generated.h"
#include "moc_protocol/Handshake_generated.h"

using namespace std::chrono_literals;

class TestClient final : public netlib::ClientBase {
public:
  TestClient()
  {
	ClientBase::Connect(
	    netlib::constants::kServerIp,
	    netlib::constants::kServerPort);
  }

  void OnConnection()
  {
	auto request = protocol::CreateHandshakeRequestDirect(
	    _fbb, 1337, "", "{1337-1337-1337}", "TestClient");

	SendPacket(protocol::MsgType_HandshakeRequest, request);
	std::puts("TestClient::OnConnection()");
  }

  void OnDisconnected(int status)
  {
	std::printf("TestClient::OnDisconnected(): %d\n", status);
  }

  void OnConsume(const uint8_t *data, size_t length)
  {
  }

  template <typename T>
  void SendPacket(protocol::MsgType tt, const flatbuffers::Offset<T> ref)
  {
	_fbb.Finish(protocol::CreateMessage(_fbb, tt, ref.Union()));

	bool result = SendReliable(_fbb.GetBufferPointer(), _fbb.GetSize());
	assert(result);
  }

private:
  flatbuffers::FlatBufferBuilder _fbb;
};

int main()
{
  netlib::ScopedNetContext context;
  TestClient client;

  while(true) {
	client.Tick();
	std::this_thread::sleep_for(1ms);
  }

  return 0;
}