// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncController.h"
#include "net/NetClient.h"

#include <qtimer.h>
#include <qglobal.h>
#include <qsettings.h>

#include "utils/Storage.h"

#include "net/protocol/Project_generated.h"
#include "net/protocol/Message_generated.h"

#include "net/NetBase.h"
#include "IdaInc.h"

#include "handlers/SyncHandler.h"

namespace noda::sync
{
	constexpr int kNetTrackRate = 1000;

	ssize_t SyncController_IdbEvent(void *userData, int code, va_list args)
	{
		return static_cast<SyncController *>(userData)->HandleEvent(hook_type_t::HT_IDB, code, args);
	}
	ssize_t SyncController_IdpEvent(void *userData, int code, va_list args)
	{
		return static_cast<SyncController *>(userData)->HandleEvent(hook_type_t::HT_IDP, code, args);
	}

	SyncController::SyncController()
	{
		hook_to_notification_point(hook_type_t::HT_IDB, SyncController_IdbEvent, this);
		hook_to_notification_point(hook_type_t::HT_IDP, SyncController_IdpEvent, this);

		_client.reset(new net::NetClient(*this));

		// post the net stats every second to the ui
		_statsTimer.reset(new QTimer(this));
		connect(_statsTimer.data(), &QTimer::timeout, ([&] {
			        emit StatsUpdated(netStats);
		        }));
		
		for (auto* i = SyncHandler::ROOT(); i;) {
			auto* it = i->handler;
			if (it) {
				LOG_TRACE("Registering handler {}", 
					protocol::EnumNameMsgType(it->msgType));

				_idaEvents[std::make_pair(it->hookType, it->hookEvent)] = it;
				_netEvents[it->msgType] = it;
			}

			auto* j = i->next;
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
		// intiialize storage?
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

	// bounce back
	void SyncController::OnConnected(int myIndex, int numUsers)
	{
		char md5[16];
		retrieve_input_file_md5(reinterpret_cast<uchar*>(md5));

		char name[128]{};
		get_root_filename(name, sizeof(name) - 1);

		// send IDB info
		auto pack = protocol::CreateLocalProjectDirect(_fbb, md5, name, 1337);
		SendFbsPacket(protocol::MsgType_LocalProject, pack);

		emit Connected();
	}

	void SyncController::OnDisconnect(uint32_t reason)
	{
		_active = false;
		_statsTimer->stop();

		// forward the event
		emit Disconnected(reason);
	}

	void SyncController::ProcessPacket(const protocol::Message* message)
	{
		switch(message->msg_type()) {
		case protocol::MsgType_RemoteProject: {
			const auto *pack = message->msg_as_RemoteProject();

			LOG_INFO(
			    "Successfully connected to {}\n"
			    "Local Version: {} Remote Version: {}",
			    pack->name()->c_str(),
			    1337,
			    pack->version());

			_active = true;
			_statsTimer->start(kNetTrackRate);
			return;
		}
		case protocol::MsgType_Broadcast: {
			const auto *pack = message->msg_as_Broadcast();
			switch(pack->type()) {
			case protocol::BroadcastType_FirstJoin:
				LOG_INFO("{} joined this project!", pack->name()->c_str());
				break;
			case protocol::BroadcastType_Joined:
				LOG_INFO("{} connected.", pack->name()->c_str());
				break;
			case protocol::BroadcastType_Disconnect:
				LOG_INFO("{} disconnected.", pack->name()->c_str());
				break;
			default:
				LOG_WARNING("Unknown broadcast announced");
				break;
			}

			emit Broadcasted(pack->type());
			return;
		}
		default: {
			auto it = _netEvents.find(message->msg_type());
			if(it != _netEvents.end()) {
				it->second->delegates.apply(*this, message);
			}
		}
		}
	}

	ssize_t SyncController::HandleEvent(hook_type_t type, int code, va_list args)
	{
		if(_active) {
			if(type == hook_type_t::HT_IDB && code == idb_event::closebase) {
				_client->Disconnect();
				return 0;
			} else if(type == hook_type_t::HT_IDP) {
				switch(code) {
				case processor_t::event_t::ev_create_func_frame:
					// TODO?
					break;
				}
			}

			const auto it = _idaEvents.find(std::make_pair(type, code));
			if(it != _idaEvents.end()) {
				it->second->delegates.react(*this, args);
			}
		}

		return 0;
	}
} // namespace noda::sync