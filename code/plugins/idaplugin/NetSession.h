// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <QObject>
#include "network/TCPClient.h"

namespace noda {

class Plugin;

class NetSession final : public QObject, public network::ClientDelegate {
 public:
  NetSession(Plugin&);
  ~NetSession();

  enum class State {
    Disconnected,
    Connecting,
    JoinedProject,
  };

 private:
  void OnConnection(const sockpp::inet_address&) override;
  void OnDisconnected(int reason) override;
  void ConsumeMessage(const uint8_t* ptr, size_t len) override;

  void HandleAuth(const protocol::MessageRoot*);

  bool _sessionActive = false;
  Plugin& _plugin;

 signals:
  void JoinedSession(int userCount);
};
}  // namespace noda