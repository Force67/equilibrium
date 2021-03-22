// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"

#include "Plugin.h"
#include "sync_session.h"
#include "sync_utils.h"

#include "utils/Logger.h"

#include <QSettings>
#include <QTimer>

SyncSession::SyncSession(Plugin& plugin) : _plugin(plugin), _service(*this) {
  _state = TransportState::DISABLED;

  _timeout.reset(new QTimer());

  connect(_timeout.data(), &QTimer::timeout, this,
          [&]() { SetTransportState(TransportState::DISABLED); });
}

SyncSession::~SyncSession() {
  _state = TransportState::DISABLED;
}

const char* SyncSession::TransportStateToString(TransportState state) {
#define TS_HELPER(x) \
  case x:            \
    return #x;
  switch (state) {
    TS_HELPER(TransportState::DISABLED);
    TS_HELPER(TransportState::INITIALIZED);
    TS_HELPER(TransportState::PENDING);
    TS_HELPER(TransportState::CONFIGURING);
    TS_HELPER(TransportState::ACTIVE);
    default:
      return "Unknown";
  }
}

const char* SyncSession::CurrentTransportStateToString() const {
  return TransportStateToString(_state);
}

void SyncSession::SetTransportState(TransportState newState) {
  if (newState != _state) {
    LOG_TRACE("Updated transport state {} -> {}",
              CurrentTransportStateToString(),
              TransportStateToString(newState));

    _state = newState;

    emit TransportStateChange(_state);
  }
}

void SyncSession::LoginUser() {
  QString name, guid = sync_utils::GetUserGuid();

  {
    QSettings settings;
    name = settings.value("Nd_SyncUser", sync_utils::GetDefaultUserName())
               .toString();
  }

  network::FbsBuffer buffer;
  auto request = protocol::CreateHandshakeRequest(
      buffer, network::kClientVersion, buffer.CreateSharedString(""),
      sync_utils::CreateFbStringRef(buffer, guid),
      sync_utils::CreateFbStringRef(buffer, name));

  _plugin.client().SendPacket(protocol::MsgType_HandshakeRequest, buffer,
                              request.Union());

  SetTransportState(TransportState::PENDING);

  // give the server 5 seconds to respond
  _timeout->start(5000);

  // TODO: think about making timeout stuff a client feature...
  _timeout->moveToThread(&_plugin.client());
}

void SyncSession::LogOff() {
  // TODO: send IQUIT
  SetTransportState(TransportState::DISABLED);
}