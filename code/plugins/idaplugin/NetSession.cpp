// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "NetSession.h"
#include "Pch.h"
#include "Plugin.h"

#include "utils/Logger.h"
#include "utils/UserInfo.h"

#include <QSettings>

namespace noda {

static network::FbsStringRef MakeFbStringRef(network::FbsBuffer& msg,
                                             const QString& other) {
  const char* str = const_cast<const char*>(other.toUtf8().data());
  size_t sz = static_cast<size_t>(other.size());
  return msg.CreateString(str, sz);
}

NetSession::NetSession(Plugin& plugin) : _plugin(plugin) {}

NetSession::~NetSession() {}

void NetSession::OnConnection(const sockpp::inet_address& addr) {
  QString name, guid = GetUserGuid();

  {
    QSettings settings;
    name = settings.value("Nd_SyncUser", GetDefaultUserName()).toString();
  }

  network::FbsBuffer buffer;
  auto request = protocol::CreateHandshakeRequest(
      buffer, network::constants::kClientVersion, buffer.CreateSharedString(""),
      MakeFbStringRef(buffer, guid), MakeFbStringRef(buffer, name));

  _plugin.client().SendPacket(protocol::MsgType_HandshakeRequest, buffer,
                              request.Union());
}

void NetSession::OnDisconnected(int reason) {}

void NetSession::HandleAuth(const protocol::MessageRoot* root) {
  const protocol::HandshakeAck* msg = root->msg_as_HandshakeAck();

  LOG_INFO("Joined session. {} users online.", msg->numUsers());

  emit JoinedSession(msg->numUsers());
}

void NetSession::ConsumeMessage(const uint8_t* ptr, size_t len) {
  const protocol::MessageRoot* message =
      protocol::GetMessageRoot(static_cast<const void*>(ptr));
  if (message->msg_type() == protocol::MsgType_HandshakeAck) {
    _sessionActive = true;
    return HandleAuth(message);
  }
}
}  // namespace noda