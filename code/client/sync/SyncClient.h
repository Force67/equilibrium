// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include "net/NetClient.h"
#include "netmessages/MsgList_generated.h"

using MsgBuilder = flatbuffers::FlatBufferBuilder;

class SyncClient final : public NetClient {
  public:
  bool Connect();

  template<typename T>
  bool SendPacket(MsgBuilder &, netmsg::Data, const T &);
};

flatbuffers::Offset<flatbuffers::String> ToFbString(MsgBuilder &, const QString &);