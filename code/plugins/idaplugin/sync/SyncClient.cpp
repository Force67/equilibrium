// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <qsettings.h>

#include "SyncClient.h"

#include "utils/UserInfo.h"
#include "utils/Logger.h"

#include "utility/ObjectPool.h"

#include "moc_protocol/Constants_generated.h"

namespace noda {

  constexpr uint32_t kNetworkerThreadIdle = 1;

  inline utility::object_pool<SyncClient::InPacket> s_inpacketPool;

  SyncClient::SyncClient(SyncDelegate &nsd) :
      _delegate(nsd)
  {
  }

  SyncClient::~SyncClient() {}

  bool SyncClient::ConnectServer()
  {
	QSettings settings;
	uint port = settings.value("Nd_SyncPort", netlib::constants::kServerPort).toUInt();
	auto addr = settings.value("Nd_SyncIp", netlib::constants::kServerIp).toString();

	bool result = Client::Connect(addr.toUtf8(), static_cast<uint16_t>(port));
	if(result) {
	  // fire event
	  _run = true;
	  QThread::start();
	}

	return result;
  }

  void SyncClient::OnConnection()
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

  void SyncClient::OnDisconnected(int r)
  {
	_delegate.OnDisconnect(r);
  }

  void SyncClient::run()
  {
	while(_run) {
	  Client::Tick();
	  QThread::msleep(kNetworkerThreadIdle);
	}
  }

  void SyncClient::OnConsume(netlib::Packet *packet)
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

  void SyncClient::HandleAuth(const protocol::Message *message)
  {
	auto *pack = message->msg_as_HandshakeAck();
  }

} // namespace noda