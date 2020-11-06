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

  inline utility::object_pool<Client::InPacket> s_inpacketPool;

  Client::Client(SyncDelegate &nsd) :
      _delegate(nsd)
  {
  }

  Client::~Client()
  {
	_run = false;
	QThread::wait();
  }

  bool Client::ConnectServer()
  {
	QSettings settings;
	uint port = settings.value("Nd_SyncPort", netlib::constants::kServerPort).toUInt();
	auto addr = settings.value("Nd_SyncIp", netlib::constants::kServerIp).toString();

	bool result = Client::Connect(addr.toUtf8().data(), static_cast<uint16_t>(port));
	if(result) {
	  // fire event
	  LOG_INFO("Connected to {}:{}", addr.toUtf8().data(), port);
	  _run = true;
	  QThread::start();
	}

	return result;
  }

  void Client::OnConnection()
  {
	QSettings settings;
	flatbuffers::FlatBufferBuilder fbb;

	const QString &guid = GetUserGuid();

	auto name = settings.value("Nd_SyncUser",
	                           GetDefaultUserName())
	                .toString();

	auto request = protocol::CreateHandshakeRequest(
	    fbb, protocol::constants::ProtocolVersion_Current,
	    fbb.CreateString(""),
	    MakeFbStringRef(fbb, guid),
	    MakeFbStringRef(fbb, name));

	//SendPacket(protocol::MsgType_HandshakeRequest, request);
  }

  void Client::OnDisconnected(int r)
  {
	_delegate.OnDisconnect(r);
  }

  void Client::run()
  {
	while(_run) {
	  Client::Tick();
	  QThread::msleep(kNetworkerThreadIdle);
	}
  }

  void Client::OnConsume(netlib::Packet *packet)
  {
	flatbuffers::Verifier verifier(packet->data(), packet->length());
	if(!protocol::VerifyMessageBuffer(verifier)) {
	  LOG_ERROR("Received corrupt data ({})", packet->length());
	  return;
	}

	const auto *message = protocol::GetMessage(static_cast<const void *>(packet->data()));
	if(message->msg_type() == protocol::MsgType_HandshakeAck) {
	  return HandleAuth(message);
	}

	InPacket *item = s_inpacketPool.construct(packet);
	_inQueue.push(&item->key);
  }

  void Client::HandleAuth(const protocol::Message *message)
  {
	auto *pack = message->msg_as_HandshakeAck();
  }

} // namespace noda