// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncController.h"
#include "net/NetClient.h"

#include <qsettings.h>

#include "utils/IdbStorage.h"
#include "utils/SysInfo.h"

#include "net/protocol/HandshakeRequest_generated.h"
#include "net/protocol/HandshakeAck_generated.h"

#include "IdaInc.h"

#include "handlers/Dispatcher.h"

namespace noda::sync
{
  SyncController::SyncController()
  {
	_client.reset(new net::NetClient(*this));

	hook_to_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
	hook_to_notification_point(hook_type_t::HT_IDP, ProcessorEvent, this);
  }

  SyncController::~SyncController()
  {
	unhook_from_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
	unhook_from_notification_point(hook_type_t::HT_IDP, ProcessorEvent, this);
  }

  bool SyncController::ConnectServer()
  {
	return _client->ConnectServer();
  }

  void SyncController::DisconnectServer()
  {
	_client->Disconnect();
  }

  bool SyncController::IsConnected()
  {
	return _client->IsConnected() && _active;
  }

  void SyncController::OnConnectRequest()
  {
	const QString &name = utils::GetSysUsername();
	const QString &hwid = utils::GetHardwareId();

	QSettings settings;
	auto user = settings.value("Nd_SyncUser", name).toString();
	auto pass = settings.value("Nd_SyncPass", "").toString();

	const auto dbVersion = 0;

	char md5[16];
	retrieve_input_file_md5(reinterpret_cast<uchar *>(md5));

	char buffer[128]{};
	get_root_filename(buffer, sizeof(buffer) - 1);

	net::FbsBuilder builder;
	auto request = protocol::CreateHandshakeRequest(
	    builder,
	    net::constants::kClientVersion,
	    net::MakeFbStringRef(builder, hwid),
	    net::MakeFbStringRef(builder, user),
	    net::MakeFbStringRef(builder, pass),
	    dbVersion,
	    builder.CreateString(md5),
	    builder.CreateString(buffer));

	_client->SendFBReliable(
	    builder,
	    protocol::MsgType_HandshakeRequest,
	    request);
  }

  void SyncController::OnDisconnect(uint32_t reason)
  {
	_active = false;
	// TODO: clean up other users...

	// forward the event
	emit Disconnected(reason);
  }

  void SyncController::ProcessPacket(uint8_t *data, size_t length)
  {
	const protocol::MessageRoot *message =
	    protocol::GetMessageRoot(static_cast<void *>(data));

	msg("Recieved message %s\n",
	    protocol::EnumNameMsgType(message->msg_type()));

	if(message->msg_type() == protocol::MsgType_HandshakeAck) {
	  const auto *ack = message->msg_as_HandshakeAck();

	  msg("We are connected %s\n", ack->project()->c_str());
	  // looks good?
	  _active = true;
	  emit Connected();
	  return;
	}
	else if (message->msg_type() == protocol::MsgType_Broadcast) {
		const auto* broadcast = message->msg_as_Broadcast();
		switch (broadcast->type()) {
		case protocol::BroadcastType_FirstJoin:
			break;
		case protocol::BroadcastType_Joined:
			break;
		case protocol::BroadcastType_Disconnect:
			break;
		default:
			break;
		}
	}

	// translate message type into handler index
	auto *applicant = GetNetApplicant(
	    message->msg_type() -
	    /*offset of handler messages*/ protocol::MsgType_sync_NameAddr);

	if(applicant) {
	  applicant(&_storage, *message);
	}
  }

  ssize_t SyncController::ProcessorEvent(void *userp, int code, va_list args)
  {
	auto &self = reinterpret_cast<SyncController &>(userp);

	return 0;
  }

  ssize_t SyncController::IdbEvent(void *userp, int code, va_list args)
  {
	auto &self = reinterpret_cast<SyncController &>(userp);

	if(code == idb_event::event_code_t::closebase) {
	  self._client->Disconnect();
	  return 0;
	}

	auto *reactor = GetReactor_IDB(0);
	if (reactor) {
	  reactor(*self._client, args);
	}

	return 0;
  }
} // namespace noda::sync  