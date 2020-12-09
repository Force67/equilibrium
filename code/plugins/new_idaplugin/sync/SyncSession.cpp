// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"

#include "Plugin.h"
#include "SyncSession.h"
#include "SyncUtils.h"

#include "utils/Logger.h"

#include <QSettings>

SyncSession::SyncSession(Plugin &plugin) :
    _plugin(plugin),
    _service(*this)
{
  _state = TransportState::DISABLED;
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

void SyncSession::LoginUser()
{
  QString name, guid = sync_utils::GetUserGuid();

  {
	QSettings settings;
	name = settings.value("Nd_SyncUser", sync_utils::GetDefaultUserName()).toString();
  }

  network::FbsBuffer buffer;
  auto request = protocol::CreateHandshakeRequest(
      buffer, network::constants::kClientVersion,
      buffer.CreateSharedString(""),
      sync_utils::CreateFbStringRef(buffer, guid),
      sync_utils::CreateFbStringRef(buffer, name));

  _plugin.client().SendPacket(protocol::MsgType_HandshakeRequest, buffer, request.Union());

  SetTransportState(TransportState::PENDING);
}

void SyncSession::HandleAuthAck(const protocol::MessageRoot *root)
{
  const protocol::HandshakeAck *msg = root->msg_as_HandshakeAck();

  LOG_INFO("Joined session. {} users online.", msg->numUsers());

  SetTransportState(TransportState::ACTIVE);
}

void SyncSession::ConsumeMessage(const uint8_t *ptr, size_t len)
{
  const protocol::MessageRoot *message = protocol::GetMessageRoot(static_cast<const void *>(ptr));
  if(message->msg_type() == protocol::MsgType_HandshakeAck) {
	return HandleAuthAck(message);
  }

  // if the message could be applied successfully we mark it in the IDB storage.
  bool result = _service.ProcessMessage(message);
  if(result) {
	_storage.BumpVersion();
  }
}