// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"

#include "Plugin.h"
#include "SyncSession.h"
#include "SyncUtils.h"

#include "utils/Logger.h"

#include <QTimer>
#include <QSettings>

SyncSession::SyncSession(Plugin &plugin) :
    _plugin(plugin),
    _service(*this)
{
  _state = TransportState::DISABLED;

  _timeout.reset(new QTimer());

  connect(_timeout.data(), &QTimer::timeout, this, [&]() {
	SetTransportState(TransportState::DISABLED);
  });
}

SyncSession::~SyncSession()
{
  _state = TransportState::DISABLED;
}

const char *SyncSession::TransportStateToString(TransportState state)
{
#define TS_HELPER(x) \
  case x:            \
	return #x;
  switch(state) {
	TS_HELPER(TransportState::DISABLED);
	TS_HELPER(TransportState::INITIALIZED);
	TS_HELPER(TransportState::PENDING);
	TS_HELPER(TransportState::CONFIGURING);
	TS_HELPER(TransportState::ACTIVE);
  default:
	return "Unknown";
  }
}

const char *SyncSession::CurrentTransportStateToString() const
{
  return TransportStateToString(_state);
}

void SyncSession::SetTransportState(TransportState newState)
{
  if(newState != _state) {
	LOG_TRACE("Updated transport state {} -> {}",
	          CurrentTransportStateToString(),
	          TransportStateToString(newState));

	_state = newState;

	emit TransportStateChange(_state);
  }
}

int SyncSession::UserCount() const
{
  return _userCount;
}

void SyncSession::LoginUser()
{
  QString name, guid = sync_utils::GetUserGuid();

  {
	QSettings settings;
	name = settings.value("Nd_SyncUser", sync_utils::GetDefaultUserName()).toString();
  }

  network::FbsBuffer buffer;
  auto request = protocol::CreateHandshakeRequest(
      buffer, network::kClientVersion,
      buffer.CreateSharedString(""),
      sync_utils::CreateFbStringRef(buffer, guid),
      sync_utils::CreateFbStringRef(buffer, name));

  _plugin.client().SendPacket(protocol::MsgType_HandshakeRequest, buffer, request.Union());

  SetTransportState(TransportState::PENDING);

  // give the server 5 seconds to respond
  _timeout->start(5000);

  // TODO: think about making timeout stuff a client feature...
  _timeout->moveToThread(&_plugin.client());
}

void SyncSession::LogOff()
{
	// TODO: send IQUIT
  SetTransportState(TransportState::DISABLED);
}

void SyncSession::HandleAuthAck(const protocol::MessageRoot *root)
{
  _timeout->stop();
  SetTransportState(TransportState::ACTIVE);

  const protocol::HandshakeAck *msg = root->msg_as_HandshakeAck();

  _userCount = msg->numUsers();
  LOG_INFO("Joined session. {} users online.", msg->numUsers());

  emit SessionNotification(NotificationCode::USERS_JOIN);
}

void SyncSession::HandleUserEvent(const protocol::MessageRoot *root)
{
  const protocol::UserEvent *msg = root->msg_as_UserEvent();

  LOG_TRACE("HandleUserEvent() -> {}",
            protocol::EnumNameUserEventType(msg->type()));

  if(msg->type() == protocol::UserEventType_Join) {
	_userCount++;
	emit SessionNotification(NotificationCode::USER_JOIN);
  }

  if(msg->type() == protocol::UserEventType_Quit) {
	if(_userCount - 1 < 1) {
	  LOG_WARNING("HandleUserEvent() -> NetCrime: less than one user not possible");
	  return;
	}

	_userCount--;
	emit SessionNotification(NotificationCode::USER_QUIT);
  }
}

void SyncSession::ConsumeMessage(const uint8_t *ptr, size_t len)
{
  const protocol::MessageRoot *message =
      protocol::GetMessageRoot(static_cast<const void *>(ptr));

  LOG_TRACE("ConsumeMessage() -> {}", protocol::EnumNameMsgType(message->msg_type()));

  switch(message->msg_type()) {
  case protocol::MsgType_HandshakeAck:
	return HandleAuthAck(message);
  case protocol::MsgType_UserEvent:
	return HandleUserEvent(message);
  default:
	break;
  }

  // if the message could be applied successfully we mark it in the IDB storage.
  bool result = _service.ProcessNetMessage(message);
  if(result) {
	_storage.BumpVersion();
  }
}