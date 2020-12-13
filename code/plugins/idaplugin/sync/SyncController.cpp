// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "TaskHandler.h"
#include "SyncController.h"

#include <QTimer>
#include <QSettings>
#include <qglobal.h>

#include "utils/UserInfo.h"
#include "protocol/generated/StorageModel_generated.h"

#include "Plugin.h"

namespace noda {
  constexpr int kNetTrackRate = 1000;

  constexpr uint32_t kStorageVersion = 4;
  static const char kSyncNodeName[] = "$ nd_sync_data";

  SyncController::SyncController(Plugin &plugin) :
      _plugin(plugin),
      _dispatcher(*this)
  {
	hook_to_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
	hook_to_notification_point(hook_type_t::HT_IDP, IdpEvent, this);

	// rather hacky but it works /shrug
	connect(this, &SyncController::Connected, [&]() {
	  struct request : exec_request_t {
		SyncController *self;

		int execute() override
		{
		  self->InitializeLocalProject();
		  return 0;
		}
	  };

	  request req;
	  req.self = this;

	  execute_sync(req, MFF_WRITE);
	});
  }

  SyncController::~SyncController()
  {
	unhook_from_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
	unhook_from_notification_point(hook_type_t::HT_IDP, IdpEvent, this);
  }

  ssize_t SyncController::IdbEvent(void *userData, int code, va_list args)
  {
	return static_cast<SyncController *>(userData)->HandleEvent(hook_type_t::HT_IDB, code, args);
  }

  ssize_t SyncController::IdpEvent(void *userData, int code, va_list args)
  {
	return static_cast<SyncController *>(userData)->HandleEvent(hook_type_t::HT_IDP, code, args);
  }

  void SyncController::InitializeLocalProject()
  {
	assert(is_main_thread());

	// load the permanent data
	_node = NetNode(kSyncNodeName, false);

	bool hasPermanentData = _node.open();

	// looks like a new IDB
	if(!hasPermanentData) {
	  _node = NetNode(kSyncNodeName, true);
	}

	_localVersion = _node.LoadScalar(NodeIndex::UpdateVersion, 0);

  }

  // recv a net message
  void SyncController::ConsumeMessage(const uint8_t *data, size_t size)
  {
	_dispatcher.QueueTask(data, size);
  }

  // dispatch net message
  ssize_t SyncController::HandleEvent(hook_type_t type, int code, va_list args)
  {
	if(!_active)
	  return 0;

	if(type == hook_type_t::HT_IDB) {
	  switch(code) {
	  case idb_event::closebase:
		break;
	  case idb_event::savebase:
		//_node.StoreScalar()
		break;
	  }

	  return 0;
	}

	if(type == hook_type_t::HT_IDP) {
	  switch(code) {
	  default:
		break;
	  }
	}

	_dispatcher.DispatchEvent(type, code, args);
	return 0;
  }
} // namespace noda