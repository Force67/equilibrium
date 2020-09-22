// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncController.h"
#include "net/NetClient.h"

#include <qsettings.h>

#include "utils/Storage.h"
#include "utils/SystemInfo.h"

#include "net/protocol/HandshakeRequest_generated.h"
#include "net/protocol/HandshakeAck_generated.h"
#include "net/protocol/Message_generated.h"

#include "net/NetBase.h"
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
	// intiialize storage?
	return _client->ConnectServer();
  }

  void SyncController::DisconnectServer()
  {
	// flush storage?
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

	SendFbsPacket(
	    builder,
	    protocol::MsgType_HandshakeRequest,
	    request);
  }

  void SyncController::OnDisconnect(uint32_t reason)
  {
	_active = false;

	// forward the event
	emit Disconnected(reason);
  }

  void SyncController::ProcessPacket(uint8_t *data, size_t length)
  {
	const protocol::Message *message =
	    protocol::GetMessage(static_cast<void *>(data));

	LOG_TRACE("Recieved message {}",
	          protocol::EnumNameMsgType(message->msg_type()));

	switch(message->msg_type()) {
	case protocol::MsgType_HandshakeAck: {
	  const auto *pack = message->msg_as_HandshakeAck();
	  _active = true;

	  LOG_INFO(
	      "Successfully connected as {} (project: {})\n"
	      "Local Version: {} Remote Version: {}",
	      pack->userName()->c_str(),
	      pack->project()->c_str(),
	      1337,
	      pack->dbRemoteVersion());

	  // everything OK! we can proceed
	  emit Connected();
	  return;
	}
	case protocol::MsgType_Broadcast: {
	  const auto *pack = message->msg_as_Broadcast();
	  switch(pack->type()) {
	  case protocol::BroadcastType_FirstJoin:
		LOG_INFO("{} joined this project!", pack->name()->c_str());
		break;
	  case protocol::BroadcastType_Joined:
		LOG_INFO("{} connected.", pack->name()->c_str());
		break;
	  case protocol::BroadcastType_Disconnect:
		LOG_INFO("{} disconnected.", pack->name()->c_str());
		break;
	  default:
		break;
	  }
	  break;
	}
	case protocol::MsgType_ChatMessage: {
	  const auto *pack = message->msg_as_ChatMessage();
	  break;
	}
	}

	// translate message type into handler index
	auto *applicant = GetNetApplicant(
	    message->msg_type() - protocol::MsgType_sync_NameEa);

	if(applicant) {
	  applicant(this, message->msg());
	}
  }

  ssize_t SyncController::ProcessorEvent(void *userp, int code, va_list args)
  {
	auto &self = reinterpret_cast<SyncController &>(userp);
	if(self._active) {
	  if(code == processor_t::event_t::ev_create_func_frame) {
	  }
	}

	return 0;
  }

  ssize_t SyncController::IdbEvent(void *userp, int code, va_list args)
  {
	auto &self = reinterpret_cast<SyncController &>(userp);

	if(code == idb_event::event_code_t::closebase) {
	  self._client->Disconnect();
	  return 0;
	}

	if(self._active) {
#if 0
	  auto *reactor = GetReactor_IDB(0);
	  if(reactor) {
		reactor(&self, args);
	  }
#endif
	}

	return 0;
  }
} // namespace noda::sync