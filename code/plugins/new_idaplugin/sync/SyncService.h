#pragma once
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

class SyncSession;

namespace protocol {
  struct MessageRoot;
}

class SyncService {
public:
  SyncService(SyncSession &);
  ~SyncService();

  bool ProcessMessage(const protocol::MessageRoot *);

private:
  void BindTaskHandlers();

  SyncSession &_session;
};