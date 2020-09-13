// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "net/NetClient.h"
#include "protocol/MsgList_generated.h"

namespace noda
{
  using MsgBuilder = flatbuffers::FlatBufferBuilder;

  class SyncClient final : public NetClient
  {
  public:
	bool Connect();
	void Disconnect();

	template <typename T>
	bool SendPacket(MsgBuilder &, protocol::Data, const T &);
  };

  flatbuffers::Offset<flatbuffers::String> ToFbString(MsgBuilder &, const QString &);
}