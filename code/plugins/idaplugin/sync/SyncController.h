// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <QThread>
#include <map>

#include "network/TCPClient.h"
#include "utils/Logger.h"
#include "utils/NetNode.h"

#include "TaskDispatcher.h"

namespace QT {
class QTimer;
}

namespace protocol {
struct MessageRoot;
}

namespace noda {
struct TaskHandler;
class Plugin;

class SyncController final : public QThread, public network::ClientDelegate {
  Q_OBJECT;

 public:
  SyncController(Plugin&);
  SyncController(const SyncController&) = delete;
  ~SyncController();

  void InitializeLocalProject();

 private:
  // network events
  void ConsumeMessage(const uint8_t* data, size_t size) override;

  // IDA
  ssize_t HandleEvent(hook_type_t, int, va_list);

  static ssize_t IdbEvent(void* userData, int code, va_list args);
  static ssize_t IdpEvent(void* userData, int code, va_list args);

 private:
  enum NodeIndex : nodeidx_t {
    UpdateVersion,
  };

  NetNode _node;

  bool _active = false;

  TaskDispatcher _dispatcher;
  int _localVersion = 0;
  int _userCount = 0;

  Plugin& _plugin;
};
}  // namespace noda