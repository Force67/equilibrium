// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#undef GetMessage

#include "Pch.h"
#include "TaskHandler.h"
#include "SyncController.h"

#include <QTimer>
#include <QSettings>
#include <qglobal.h>

#include "utils/UserInfo.h"
#include "protocol/generated/StorageModel_generated.h"

namespace noda {
  constexpr int kNetTrackRate = 1000;

  constexpr uint32_t kStorageVersion = 4;
  static const char kSyncNodeName[] = "$ nd_sync_data";

  static network::FbsStringRef MakeFbStringRef(network::FbsBuffer &msg, const QString &other)
  {
	const char *str = const_cast<const char *>(other.toUtf8().data());
	size_t sz = static_cast<size_t>(other.size());
	return msg.CreateString(str, sz);
  }

  SyncController::SyncController() :
      _client(*this),
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

  bool SyncController::Connect()
  {
	QSettings settings;
	int port = settings.value("Nd_SyncPort", network::constants::kServerPort).toInt();
	auto addr = settings.value("Nd_SyncIp", network::constants::kServerIp).toString();

	if(_client.Connect(
	       addr.toUtf8().data(),
	       static_cast<int16_t>(port))) {
	  LOG_INFO("Connected to {}:{}", addr.toUtf8().data(), port);

	  auto name = settings.value("Nd_SyncUser", GetDefaultUserName()).toString();
	  auto guid = GetUserGuid();

	  network::FbsBuffer buffer;
	  auto request = protocol::CreateHandshakeRequest(
	      buffer, network::constants::kClientVersion,
	      buffer.CreateSharedString(""),
	      MakeFbStringRef(buffer, guid),
	      MakeFbStringRef(buffer, name));

	  _client.SendPacket(protocol::MsgType_HandshakeRequest, buffer, request.Union());
	  return true;
	}

	LOG_ERROR("Failed to connect to {}:{}", addr.toUtf8().data(), port);
	return false;
  }

  void SyncController::InitializeLocalProject()
  {
	assert(is_main_thread());

	// Note that this is not invoked from net thread..
	_node = NetNode(kSyncNodeName);
	if(!_node.good()) {
	  LOG_ERROR("Bad node {}", kSyncNodeName);
	  return;
	}

	_localVersion = _node.LoadScalar(NodeIndex::UpdateVersion, 0);

	uchar md5[16]{};
	bool result = retrieve_input_file_md5(md5);
	assert(result != false);

	char md5Str[32 + 1]{};

	// convert bytes to str
	constexpr char lookup[] = "0123456789abcdef";
	for(int i = 0; i < 16; i++) {
	  md5Str[i * 2] = lookup[(md5[i]) >> 4 & 0xF];
	  md5Str[i * 2 + 1] = lookup[(md5[i]) & 0xF];
	}

	char fileName[128]{};
	get_root_filename(fileName, sizeof(fileName) - 1);

	network::FbsBuffer buffer;
	auto request = protocol::CreateLocalProjectInfoDirect(
	    buffer, md5Str, fileName, _localVersion);

	_client.SendPacket(protocol::MsgType_LocalProjectInfo, buffer, request.Union());
  }

  void SyncController::OnDisconnect(int reason)
  {
	LOG_INFO("Disconnected with reason {}", reason);

	_active = false;

	// forward the event
	emit Disconnected(reason);
  }

  void SyncController::HandleAuth(const protocol::MessageRoot *message)
  {
	auto *pack = message->msg_as_HandshakeAck();

	LOG_INFO("Authenticated: {}/{}", pack->userIndex(), pack->numUsers());

	_active = true;
	_userCount = pack->numUsers();

	emit Connected();

	emit Announce(_userCount);
  }

  // recv a net message
  void SyncController::ConsumeMessage(const uint8_t *data, size_t size)
  {
	const protocol::MessageRoot *message = protocol::GetMessageRoot(static_cast<const void *>(data));
	if(message->msg_type() == protocol::MsgType_HandshakeAck) {
	  return HandleAuth(message);
	}

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
		Disconnect();
		break;
	  case idb_event::savebase:

		break;
	  }

	  return 0;
	}

	_dispatcher.DispatchEvent(type, code, args);
	return 0;
  }
} // namespace noda