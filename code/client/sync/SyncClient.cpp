// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include <qsettings.h>
#include "SyncClient.h"
#include "utility/SysInfo.h"
#include <nalt.hpp>

#include "net/NetBuffer.h"
#include "netmessages/Handshake_generated.h"

#include "IdaInc.h"

#if 0
//https://google.github.io/flatbuffers/md__schemas.html
//https://stackoverflow.com/questions/37685003/flatbuffers-send-multiple-packet-types-using-a-union
//https://groups.google.com/forum/#!topic/flatbuffers/3Oe-jYbOeQY
//https://github.com/google/pienoon/blob/master/src/flatbufferschemas/multiplayer.fbs
//https://github.com/google/pienoon/blob/a721180de4d9de3696af3b3734d1ad269975b111/src/pie_noon_game.cpp#L2165
#endif

// fast conversion
flatbuffers::Offset<flatbuffers::String> ToFbString(MsgBuilder &msg, const QString &other) {
  const char *str = const_cast<const char *>(other.toUtf8().data());
  size_t sz = static_cast<size_t>(other.size());
  return msg.CreateString(str, sz);
}

bool SyncClient::Connect() {
  QSettings settings;
  auto ip = settings.value("NODASyncIp", kServerIp).toString();
  uint port = settings.value("NODASyncPort", kServerPort).toUInt();

  // this blocks until the connection is established
  if (! NetClient::Connect(ip.toUtf8().data(), static_cast<uint16_t>(port))) {
	msg("LMAO");
	return false;
  }

  const QString &sysName = utility::GetSysUsername();
  const QString &sysHwid = utility::GetHardwareId();
  auto user = settings.value("NODASyncUser", sysName).toString();
  auto pass = settings.value("NODASyncPass", "").toString();

  const auto dbVersion = 0;

  char md5[16];
  retrieve_input_file_md5(reinterpret_cast<uchar *>(md5));

  char buffer[128]{};
  get_root_filename(buffer, sizeof(buffer) - 1);

  MsgBuilder builder;
  auto request = netmsg::CreateHandshake(
	  builder, kClientVersion,
	  ToFbString(builder, sysHwid),
	  ToFbString(builder, user),
	  ToFbString(builder, pass),
	  dbVersion,
	  builder.CreateString(md5),
	  builder.CreateString(buffer));

  if (! SendPacket(builder, netmsg::Data::Data_Handshake, request)) {
	NetClient::Disconnect();
	return false;
  }

  return true;
}

template <typename T>
bool SyncClient::SendPacket(MsgBuilder &mb, netmsg::Data type, const T &data) {
  auto msgRoot = netmsg::CreateMessageRoot(mb, type, data.Union());
  mb.Finish(msgRoot);

  return NetClient::SendReliable(mb.GetBufferPointer(), mb.GetSize());
}
