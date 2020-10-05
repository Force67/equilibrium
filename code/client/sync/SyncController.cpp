// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncController.h"
#include "net/NetClient.h"

#include <qtimer.h>
#include <qglobal.h>
#include <qsettings.h>

#include "utils/Storage.h"

#include "net/protocol/Workspace_generated.h"
#include "net/protocol/Message_generated.h"

#include "net/NetBase.h"
#include "IdaInc.h"

#include "SyncHandler.h"

namespace noda {
  constexpr int kNetTrackRate = 1000;

  RequestItem::RequestItem(SyncHandler *handler, size_t bucketSize) :
      handler(handler)
  {
	data = std::make_unique<uint8_t[]>(bucketSize);
  }

  int RequestQueue::execute()
  {
	RequestItem *item;
	while(_queue.pop(item)) {
	  _queueLength--;

	  // drain the queue
	  item->handler->delegates.apply(
	      parent,
	      item->data.get());

	  item->data.reset();
	}

	return 0;
  }

  void RequestQueue::Queue(RequestItem *data)
  {
	_queueLength++;
	_queue.push(data);

	if(_queueLength == 1) {
	  execute_sync(*this, MFF_WRITE | MFF_NOWAIT);
	}
  }

  ssize_t SyncController_IdbEvent(void *userData, int code, va_list args)
  {
	return static_cast<SyncController *>(userData)->HandleEvent(hook_type_t::HT_IDB, code, args);
  }
  ssize_t SyncController_IdpEvent(void *userData, int code, va_list args)
  {
	return static_cast<SyncController *>(userData)->HandleEvent(hook_type_t::HT_IDP, code, args);
  }

  SyncController::SyncController() :
      _requestQueue(*this)
  {
	hook_to_notification_point(hook_type_t::HT_IDB, SyncController_IdbEvent, this);
	hook_to_notification_point(hook_type_t::HT_IDP, SyncController_IdpEvent, this);

	_client.reset(new NetClient(*this));

	// post the net stats every second to the ui
	_statsTimer.reset(new QTimer(this));
	connect(_statsTimer.data(), &QTimer::timeout, ([&] {
	          emit StatsUpdated(netStats);
	        }));

	for(auto *i = SyncHandler::ROOT(); i;) {
	  auto *it = i->handler;
	  if(it) {
		//LOG_TRACE("Registering handler {}",
		//	protocol::EnumNameMsgType(it->msgType));

		_idaEvents[std::make_pair(it->hookType, it->hookEvent)] = it;
		_netEvents[it->msgType] = it;
	  }

	  auto *j = i->next;
	  i->next = nullptr;
	  i = j;
	}
  }

  SyncController::~SyncController()
  {
	unhook_from_notification_point(hook_type_t::HT_IDB, SyncController_IdbEvent, this);
	unhook_from_notification_point(hook_type_t::HT_IDP, SyncController_IdpEvent, this);
  }

  bool SyncController::ConnectServer()
  {
	// initialize storage?
	return _client->ConnectServer();
  }

  void SyncController::DisconnectServer()
  {
	// flush storage?
	_client->Disconnect();
  }

  bool SyncController::IsConnected()
  {
	return _client->IsConnected() && _active;
  }

  void SyncController::OnConnected()
  {
	struct request : exec_request_t {
	  SyncController &sc;

	  int execute() override
	  {
		uchar md5[16]{};
		bool result = retrieve_input_file_md5(md5);
		assert(result != false);

		// +1 for null termination
		char md5Str[32 + 1]{};

		// convert bytes to str
		constexpr char klookup[] = "0123456789abcdef";
		for(int i = 0; i < 16; i++) {
		  md5Str[i * 2] = klookup[md5[i] / 16];
		  md5Str[i * 2 + 1] = klookup[md5[i] % 16];
		}

		char fileName[128]{};
		get_root_filename(fileName, sizeof(fileName) - 1);

		sc.SendFbsPacket(protocol::MsgType_LocalProjectInfo,
		                 protocol::CreateLocalProjectInfoDirect(
		                     sc.fbb(), 1337,
		                     md5Str, fileName));
		return 0;
	  }

	  request(SyncController &sc) :
	      sc(sc) {}
	};

	// until this completes, the server wont respond anyway
	request req(*this);
	execute_sync(req, MFF_READ);

	emit Connected();
  }

  void SyncController::OnDisconnect(uint32_t reason)
  {
	_active = false;
	_statsTimer->stop();

	// forward the event
	emit Disconnected(reason);
  }

  void SyncController::OnAnnouncement(const protocol::Message *message)
  {
	const auto *pack = message->msg_as_Announcement();
	switch(pack->type()) {
	case protocol::AnnounceType_FirstJoin:
	  LOG_INFO("{} joined this project!", pack->name()->c_str());
	  break;
	case protocol::AnnounceType_Joined:
	  LOG_INFO("{} connected.", pack->name()->c_str());
	  break;
	case protocol::AnnounceType_Disconnect:
	  LOG_INFO("{} disconnected.", pack->name()->c_str());
	  break;
	default:
	  LOG_WARNING("Unknown broadcast announced");
	  break;
	}

	emit Broadcasted(pack->type());
  }

  void SyncController::OnProjectJoin(const protocol::Message *message)
  {
	const auto *pack = message->msg_as_RemoteProjectInfo();

	if(pack->version() == 0)
	  LOG_INFO("Successfully joined {} for the first time", pack->name()->c_str());
	else {
	  LOG_INFO(
	      "Successfully connected to {}\n"
	      "Local Version: {} Remote Version: {}",
	      pack->name()->c_str(),
	      1337,
	      pack->version());
	}

	_active = true;
	_statsTimer->start(kNetTrackRate);
  }

  void SyncController::ProcessPacket(const uint8_t *data, size_t size)
  {
	const auto *message = protocol::GetMessage(data);

	switch(message->msg_type()) {
	case protocol::MsgType_RemoteProjectInfo:
	  OnProjectJoin(message);
	  return;
	case protocol::MsgType_Announcement:
	  OnAnnouncement(message);
	  return;
	default:
	  break;
	}

	auto it = _netEvents.find(message->msg_type());
	if(it == _netEvents.end())
	  return;

	// this is kinda ugly for now :slight_smile:
	auto *item = new RequestItem(it->second, size);
	std::memcpy(item->data.get(), data, size);
	_requestQueue.Queue(item);
  }

  ssize_t SyncController::HandleEvent(hook_type_t type, int code, va_list args)
  {
	if(!_active)
	  return 0;

	if(type == hook_type_t::HT_IDB) {
	}

	if(_active) {
	  if(type == hook_type_t::HT_IDB && code == idb_event::closebase) {
		_client->Disconnect();
		return 0;
	  }

	  const auto it = _idaEvents.find(std::make_pair(type, code));
	  if(it != _idaEvents.end()) {
		it->second->delegates.react(*this, args);
	  }
	}

	return 0;
  }
} // namespace noda::sync