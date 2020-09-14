// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "net/NetClient.h"
#include "protocol/MsgList_generated.h"

namespace noda
{
  using FbsBuilder = flatbuffers::FlatBufferBuilder;
  using FbsStringRef = flatbuffers::Offset<flatbuffers::String>;

  class SyncClient final : public NetClient
  {
  public:
	bool Connect();
	void Disconnect();

	template <typename T>
	bool SendPacket(FbsBuilder &, protocol::Data, const T &);
  };

  FbsStringRef ToFbString(FbsBuilder &, const QString &);
}