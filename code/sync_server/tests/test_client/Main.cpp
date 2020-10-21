// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>

#include <netlib/NetLib.h>
#include <netlib/Client.h>

// stupid windows
#undef GetMessage
#undef GetMessageW

#include "flatbuffers/flatbuffers.h"

#include "moc_protocol/Message_generated.h"
#include "moc_protocol/Handshake_generated.h"
#include "moc_protocol/IdaSync_generated.h"

#include "Packet.h"

#include "utility/Thread.h"

using namespace std::chrono_literals;

inline utility::object_pool<InPacket> packetPool;

class TestClient final : public netlib::Client {
public:
  TestClient()
  {
	_run = Client::Connect(
	    netlib::constants::kServerIp,
	    netlib::constants::kServerPort);

	if(_run) {
	  _workerThread = std::thread(&TestClient::WorkerThread, this);
	}
  }

  ~TestClient()
  {
	_workerThread.join();
  }

  void WorkerThread()
  {
	utility::SetCurrentThreadPriority(utility::ThreadPriority::High);
	utility::SetCurrentThreadName("[WorkerThread]");

	while(_run) {
	  while(auto *item = _packetQueue.pop(&InPacket::key)) {
		const netlib::Packet &packet = item->packet;

		const auto *message = protocol::GetMessage(static_cast<const void *>(packet.data()));

		std::printf("Received message: %s\n", protocol::EnumNameMsgType(message->msg_type()));
		packetPool.destruct(item);
	  }
	}
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
	/* Client -> HandshakeRequest
	Server -> HandshakeAck
	Client -> LocalProjectInfo
	Server -> WorkspaceList */

	auto *pack = message->msg_as_HandshakeAck();

	std::printf("HandleAuth(): %d:%d\n", pack->userIndex(), pack->numUsers());

	auto packet = protocol::CreateLocalProjectInfoDirect(_fbb, 1337,
	                                                     "FakeProject", "48fac413577bee5e05429a1aeda48b52");

	SendPacket(protocol::MsgType_LocalProjectInfo, packet);
  }

  void OnConsume(netlib::Packet *packet)
  {
	flatbuffers::Verifier verifier(packet->data(), packet->length());
	if(!protocol::VerifyMessageBuffer(verifier)) {
	  std::puts("Received corrupt data!");
	  return;
	}

	const auto *message = protocol::GetMessage(static_cast<const void *>(packet->data()));
	if(message->msg_type() == protocol::MsgType_HandshakeAck) {
	  return HandleAuth(message);
	}

	InPacket *item = packetPool.construct(packet);
	_packetQueue.push(&item->key);
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
	return _run;
  }

private:
  flatbuffers::FlatBufferBuilder _fbb;
  bool _run = false;

  utility::detached_mpsc_queue<InPacket> _packetQueue;

  std::thread _workerThread;
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