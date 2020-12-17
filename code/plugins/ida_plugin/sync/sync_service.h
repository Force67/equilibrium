#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"

#include <QObject>
#include <map>

class SyncSession;

namespace protocol {
struct MessageRoot;
}

namespace sync {
struct StaticHandler;
}

// stats timer..
struct SyncStats {
  int idaCount = 0;
  int netCount = 0;
};

class SyncService : QObject, exec_request_t {
  Q_OBJECT;

 public:
  SyncService(SyncSession&);
  ~SyncService();

  bool ProcessNetMessage(const protocol::MessageRoot*);

  const SyncStats& Stats() const;

 private:
  void HandleIDAEvent(hook_type_t, int code, va_list args);

  void BindStaticHandlers();
  void SendSessionInfo();

  static ssize_t IdbEvent(void* userData, int code, va_list args);
  static ssize_t IdpEvent(void* userData, int code, va_list args);

  int execute() override;

  SyncSession& _session;
  SyncStats _stats;
  bool _active = false;

  using IdaEventType_t = std::pair<hook_type_t, int>;
  std::map<IdaEventType_t, sync::StaticHandler*> _idaEvents;
  std::map<int, sync::StaticHandler*> _netEvents;
};