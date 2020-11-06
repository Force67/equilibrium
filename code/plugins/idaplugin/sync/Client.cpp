// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <qsettings.h>

#include "Client.h"

#include "utils/UserInfo.h"
#include "utils/Logger.h"
#include "utility/ObjectPool.h"

#include "moc_protocol/Constants_generated.h"

namespace noda {

  constexpr uint32_t kNetworkerThreadIdle = 1;

  inline utility::object_pool<OutPacket> s_packetPool;

  Client::Client(SyncDelegate &nsd) :
      _delegate(nsd)
  {
	NetClient::SetDeleter([](void *userp) {
	  OutPacket *out = static_cast<OutPacket *>(userp);
	  s_packetPool.destruct(out);
	});
  }

  Client::~Client()
  {
	if(_run) {
	  _run = false;
	  QThread::wait();
	  Disconnect();
	}
  }

  bool Client::Start()
  {
	QSettings settings;
	uint port = settings.value("Nd_SyncPort", netlib::constants::kServerPort).toUInt();
	auto addr = settings.value("Nd_SyncIp", netlib::constants::kServerIp).toString();

	bool result = NetClient::Connect(addr.toUtf8().data(), static_cast<uint16_t>(port));
	if(result) {
	  // fire event
	  LOG_INFO("Connecting to {}:{}", addr.toUtf8().data(), port);
	  _run = true;

	  QThread::setObjectName("[NetThread]");
	  QThread::start();
	}

	return result;
  }

  void Client::Stop()
  {
	_run = false;
	QThread::wait();

	OnDisconnected(0);
	Disconnect();
  }

  void Client::CreatePacket(protocol::MsgType type,
                            FbsBuffer &buffer,
                            FbsRef<void> packet)
  {
	buffer.Finish(protocol::CreateMessage(buffer, type, packet));

	OutPacket *item = s_packetPool.construct();
	item->buffer = std::move(buffer);

	_outQueue.push(&item->key);
  }

  void Client::OnConnection()
  {
	const QString &guid = GetUserGuid();

	QSettings settings;
	auto name = settings.value("Nd_SyncUser",
	                           GetDefaultUserName())
	                .toString();

	FbsBuffer buffer;
	auto request = protocol::CreateHandshakeRequest(
	    buffer, protocol::constants::ProtocolVersion_Current,
	    buffer.CreateString(""),
	    MakeFbStringRef(buffer, guid),
	    MakeFbStringRef(buffer, name));

	CreatePacket(protocol::MsgType_HandshakeRequest, buffer, request.Union());
  }

  void Client::OnDisconnected(int r)
  {
	_delegate.OnDisconnect(r);
  }

  void Client::run()
  {
	while(_run) {
	  NetClient::Tick();

	  while(auto *packet = _outQueue.pop(&OutPacket::key)) {
		bool result = NetClient::SendReliableUnsafe(
		    packet->buffer.GetBufferPointer(),
		    packet->buffer.GetSize(),
		    static_cast<void *>(packet));

		if(!result)
		  LOG_TRACE("Failed to send packet! ({})", packet->buffer.GetSize());
	  }

	  QThread::msleep(kNetworkerThreadIdle);
	}
  }

  void Client::OnConsume(netlib::Packet *packet)
  {
	flatbuffers::Verifier verifier(packet->data(), packet->length());
	if(!protocol::VerifyMessageBuffer(verifier)) {
	  LOG_ERROR("Received corrupt packet ({})", packet->length());
	  return;
	}

	const auto *message = protocol::GetMessage(static_cast<const void *>(packet->data()));
	if(message->msg_type() == protocol::MsgType_HandshakeAck) {
	  return HandleAuth(message);
	}

	_delegate.ProcessPacket(packet);
  }

  void Client::HandleAuth(const protocol::Message *message)
  {
	auto *pack = message->msg_as_HandshakeAck();

	LOG_INFO("Authenticated: {}/{}", pack->userIndex(), pack->numUsers());

	_delegate.OnConnected();
  }

} // namespace noda