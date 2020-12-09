// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "SessionStore.h"
#include "utils/Logger.h"

// https://github.com/chromium/chromium/blob/f0993fecf778c3c9e4758049ae1a8ffa682eed29/components/sync_sessions/session_store.cc
// https://github.com/chromium/chromium/blob/master/components/sync_sessions/session_sync_prefs.cc

namespace {

  // unique identifier for IDB storage
  constexpr char kSessionNodeId[] = "$ noda_session";

  // increment this whenever a breaking change happens
  constexpr int kSessionStoreVersion = 1;

  enum NodeIndex : nodeidx_t {
	StorageVersion,
	SessionVersion,
  };
} // namespace

SessionStore::SessionStore()
{
  // the best way is still simply to hook into the ui Event.
  hook_to_notification_point(hook_type_t::HT_UI, Event, this);
}

SessionStore::~SessionStore()
{
  unhook_from_notification_point(hook_type_t::HT_UI, Event, this);
}

void SessionStore::BumpVersion()
{
  _localVersion++;
}

void SessionStore::InitializeNode()
{
  _node = noda::NetNode(kSessionNodeId);

  int version = _node.LoadScalar<int>(NodeIndex::StorageVersion, -1);
  if(version < kSessionStoreVersion) {
	// apply change set..
  }

  // no node exists yet..
  if(version == -1) {
	_node.StoreScalar(NodeIndex::StorageVersion, kSessionStoreVersion);
  }

  _localVersion = _node.LoadScalar<int>(NodeIndex::SessionVersion, 0);
  LOG_TRACE("SessionStore::InitializeNode() -> {} : {}", version, _localVersion);
}

void SessionStore::SaveNode()
{
  // attempt to rescue the node
  if(!_node.open()) {
	InitializeNode();
  }

  bool res = _node.StoreScalar(NodeIndex::SessionVersion, _localVersion);
  LOG_TRACE("SessionStore::SaveNode() -> {}", res);
}

ssize_t SessionStore::Event(void *ptr, int code, va_list)
{
  SessionStore *store = reinterpret_cast<SessionStore *>(ptr);

  if(code == ui_notification_t::ui_saving) {
	store->SaveNode();
  }

  if(code == ui_notification_t::ui_database_inited) {
	store->InitializeNode();
  }

  return 0;
}
