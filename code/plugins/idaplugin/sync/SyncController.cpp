// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <qtimer.h>
#include <qglobal.h>
#include <qsettings.h>

#include "SyncController.h"
#include "moc_protocol/Message_generated.h"

#include "IdaInc.h"
#include "SyncHandler.h"

#include "utility/ObjectPool.h"

namespace noda {
  constexpr int kNetTrackRate = 1000;

  constexpr uint32_t kStorageVersion = 4;
  static const char kSyncNodeName[] = "$ nd_sync_data";

  static inline utility::object_pool<InPacket> s_packetPool;

  SyncController::SyncController() :
      _client(*this),
      _dispatcher(*this)
  {
	hook_to_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
	hook_to_notification_point(hook_type_t::HT_IDP, IdpEvent, this);

	for(auto *i = SyncHandler::ROOT(); i;) {
	  if(SyncHandler *it = i->handler) {
		_idaEvents[std::make_pair(it->hookType, it->hookEvent)] = it;
		_netEvents[it->msgType] = it;
	  }

	  auto *j = i->next;
	  i->next = nullptr;
	  i = j;
	}

	//_node = NetNode(kSyncNodeName, true);
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

  bool SyncController::CreateLocalHost()
  {
	/*bool result = _server.Start();
	if(result) {
	  LOG_INFO("Local server listening on port {}",
	           _server.GetPort());
	}

	return result;*/

	return false;
  }

  void SyncController::DestroyLocalHost()
  {
	//_server.Stop();
	LOG_INFO("Stopped Localhost");
  }

  bool SyncController::Connect()
  {
	// initialize storage?
	return _client.Start();
  }

  void SyncController::Disconnect()
  {
	// flush storage?
	_client.Stop();
  }

  bool SyncController::IsConnected()
  {
	return _client.Good() && _active;
  }

  bool SyncController::IsLocalHosting()
  {
	//return _server.Active();
	return false;
  }

  void SyncController::OnConnected()
  {
	_active = true;

	emit Connected();
  }

  void SyncController::OnDisconnect(int reason)
  {
	LOG_INFO("Disconnected with reason {}", reason);

	_active = false;

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
  }

  int SyncController::Dispatcher::execute()
  {
	while(InPacket *item = sc._queue.pop(&InPacket::key)) {
	  sc._eventSize--;

	  const protocol::Message *message =
	      protocol::GetMessage(item->packet.data());

	  switch(message->msg_type()) {
	  case protocol::MsgType_RemoteProjectInfo:
		sc.OnProjectJoin(message);
		return 0;
	  case protocol::MsgType_Announcement:
		sc.OnAnnouncement(message);
		return 0;
	  default:
		break;
	  }

	  auto it = sc._netEvents.find(message->msg_type());
	  if(it == sc._netEvents.end())
		return 0;

	  it->second->delegates.apply(sc, message->msg());
	  s_packetPool.destruct(item);
	}

	return 0;
  }

  void SyncController::ProcessPacket(netlib::Packet *packet)
  {
	InPacket *item = s_packetPool.construct(packet);
	_queue.push(&item->key);
	_eventSize++;

	if(_eventSize == 1)
	  execute_sync(_dispatcher, MFF_WRITE | MFF_NOWAIT);
  }

  ssize_t SyncController::HandleEvent(hook_type_t type, int code, va_list args)
  {
	if(!_active)
	  return 0;

	if(type == hook_type_t::HT_IDB && code == idb_event::closebase) {
	  _client.Disconnect();
	  return 0;
	}

	const auto it = _idaEvents.find(std::make_pair(type, code));
	if(it != _idaEvents.end()) {
	  it->second->delegates.react(*this, args);
	}

	return 0;
  }
} // namespace noda