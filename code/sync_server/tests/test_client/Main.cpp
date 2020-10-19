// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>
#include <netlib/NetLib.h>
#include <netlib/Client.h>

#include "flatbuffers/flatbuffers.h"

#include "moc_protocol/Message_generated.h"
#include "moc_protocol/Handshake_generated.h"

#include "moc_protocol/IdaSync_generated.h"

using namespace std::chrono_literals;

class TestClient final : public netlib::Client {
public:
  TestClient()
  {
	Client::Connect(
	    netlib::constants::kServerIp,
	    netlib::constants::kServerPort);
  }

  void OnConnection()
  {
	auto request = protocol::CreateHandshakeRequestDirect(
	    _fbb, 1337, "", "{1337-1337-1337}", "TestClient");

	SendPacket(protocol::MsgType_HandshakeRequest, request);
	std::puts("OnConnection()");
  }

  void OnDisconnected(int status)
  {
	std::printf("OnDisconnected(): %d\n", status);
	//_signalQuit = true;
  }

  void HandleAuth(const protocol::Message *message)
  {
	auto *pack = message->msg_as_HandshakeAck();

	std::printf("HandleAuth(): %d:%d\n", pack->userIndex(), pack->numUsers());

	// send a few fake messages
	auto request = protocol::sync::CreateNameEaDirect(_fbb, 1337, "SomeFakeName");
	SendPacket(protocol::MsgType_sync_NameEa, request);
  }

  void OnConsume(const uint8_t *data, size_t length)
  {
	flatbuffers::Verifier verifier(data, length);
	if(!protocol::VerifyMessageBuffer(verifier)) {
	  std::puts("Received corrupt data!");
	  return;
	}

	const auto *message = protocol::GetMessage(static_cast<const void *>(data));
	if(message->msg_type() == protocol::MsgType_HandshakeAck) {
	  return HandleAuth(message);
	}
  }

  template <typename T>
  void SendPacket(protocol::MsgType tt, const flatbuffers::Offset<T> ref)
  {
	_fbb.Finish(protocol::CreateMessage(_fbb, tt, ref.Union()));

	bool result = SendReliable(_fbb.GetBufferPointer(), _fbb.GetSize());
	assert(result);
  }

  bool ShouldRun() const
  {
	return !_signalQuit;
  }

private:
  flatbuffers::FlatBufferBuilder _fbb;
  bool _signalQuit = false;
};

int main()
{
  netlib::ScopedNetContext context;
  TestClient client;

  while(client.ShouldRun()) {
	client.Tick();
	std::this_thread::sleep_for(1ms);
  }

  return 0;
}