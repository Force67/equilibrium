// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncController.h"
#include "net/NetClient.h"

#include <qtimer.h>
#include <qglobal.h>
#include <qsettings.h>

#include "utils/Storage.h"
#include "utils/SystemInfo.h"

#include "net/protocol/HandshakeRequest_generated.h"
#include "net/protocol/HandshakeAck_generated.h"
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
		_client.reset(new net::NetClient(*this));

		// post the net stats every second to the ui
		_statsTimer.reset(new QTimer(this));
		connect(_statsTimer.data(), &QTimer::timeout, ([&] {
			        emit StatsUpdated(netStats);
		        }));

		hook_to_notification_point(hook_type_t::HT_IDB, SyncController_IdbEvent, this);
		hook_to_notification_point(hook_type_t::HT_IDP, SyncController_IdpEvent, this);

		auto init = detail::SyncHandler_InitDelegate::ROOT()->next;
		while(init && init->handler) {
			auto handler = init->handler;
			_idaEvents[std::make_pair(handler->hookType, handler->hookEvent)] = handler;
			_netEvents[handler->msgType] = handler;

			init = init->next;
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

	void SyncController::OnConnectRequest()
	{
		const QString &name = utils::GetSysUsername();
		const QString &hwid = utils::GetHardwareId();

		QSettings settings;
		auto user = settings.value("Nd_SyncUser", name).toString();
		auto pass = settings.value("Nd_SyncPass", "").toString();

		const auto dbVersion = 0;

		char md5[16];
		retrieve_input_file_md5(reinterpret_cast<uchar *>(md5));

		char buffer[128]{};
		get_root_filename(buffer, sizeof(buffer) - 1);

		auto request = protocol::CreateHandshakeRequest(
		    _fbb,
		    net::constants::kClientVersion,
		    net::MakeFbStringRef(_fbb, hwid),
		    net::MakeFbStringRef(_fbb, user),
		    net::MakeFbStringRef(_fbb, pass),
		    dbVersion,
		    _fbb.CreateString(md5),
		    _fbb.CreateString(buffer));

		SendFbsPacket(
		    protocol::MsgType_HandshakeRequest,
		    request);
	}

	void SyncController::OnDisconnect(uint32_t reason)
	{
		_active = false;
		_statsTimer->stop();

		// forward the event
		emit Disconnected(reason);
	}

	void SyncController::ProcessPacket(uint8_t *data, size_t length)
	{
		const protocol::Message *message =
		    protocol::GetMessage(static_cast<void *>(data));

		LOG_TRACE("Recieved message {}",
		          protocol::EnumNameMsgType(message->msg_type()));

		switch(message->msg_type()) {
		case protocol::MsgType_HandshakeAck: {
			const auto *pack = message->msg_as_HandshakeAck();
			_active = true;

			LOG_INFO(
			    "Successfully connected as {} (project: {})\n"
			    "Local Version: {} Remote Version: {}",
			    pack->userName()->c_str(),
			    pack->project()->c_str(),
			    1337,
			    pack->dbRemoteVersion());

			// everything OK! we can proceed

			_statsTimer->start(kNetTrackRate);
			emit Connected();
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
			break;
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