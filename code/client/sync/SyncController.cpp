// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncController.h"
#include "net/NetClient.h"

#include <qsettings.h>
#include "utility/SysInfo.h"

namespace noda::sync
{
  SyncController::SyncController()
  {
	_client.reset(new net::NetClient(*this));

	hook_to_notification_point(hook_type_t::HT_IDB, OnIdaEvent, this);
	hook_to_notification_point(hook_type_t::HT_IDP, OnIdaEvent, this);
  }

  SyncController::~SyncController()
  {
	unhook_from_notification_point(hook_type_t::HT_IDB, OnIdaEvent, this);
	unhook_from_notification_point(hook_type_t::HT_IDP, OnIdaEvent, this);
  }

  bool SyncController::ConnectServer()
  {
	return _client->ConnectServer();
  }

  void SyncController::DisconnectServer()
  {
	_client->Disconnect();
  }

  bool SyncController::IsConnected()
  {
	return _client->IsConnected();
  }

  void SyncController::OnConnectRequest()
  {
	const QString &userName = utility::GetSysUsername();
	const QString &hwid = utility::GetHardwareId();

	QSettings settings;
	auto user = settings.value("Nd_SyncUser", userName).toString();
	auto pass = settings.value("Nd_SyncPass", "").toString();

	const auto dbVersion = 0;

	char md5[16];
	retrieve_input_file_md5(reinterpret_cast<uchar *>(md5));

	char buffer[128]{};
	get_root_filename(buffer, sizeof(buffer) - 1);

	net::FbsBuilder builder;
	auto request = protocol::CreateHandshake(
	    builder,
	    net::constants::kClientVersion,
	    net::MakeFbStringRef(builder, hwid),
	    net::MakeFbStringRef(builder, user),
	    net::MakeFbStringRef(builder, pass),
	    dbVersion,
	    builder.CreateString(md5),
	    builder.CreateString(buffer));

	_client->SendFBReliable(
	    builder,
	    protocol::Data::Data_Handshake,
	    request);
  }

  void SyncController::OnDisconnect(uint32_t reason)
  {
	// forward the event
	emit Disconnected(reason);
  }

  void SyncController::OnConnect()
  {
	emit Connected();
  }

  void SyncController::ProcessPacket(uint8_t *data, size_t length)
  {
	const protocol::MessageRoot *message =
	    protocol::GetMessageRoot(static_cast<void *>(data));

	// find a dispatcher

  }

  ssize_t SyncController::OnIdaEvent(void *userp, int code, va_list args)
  {
	auto *self = reinterpret_cast<SyncController *>(userp);

	// find a dispatcher

	return 0;
  }
} // namespace noda::sync