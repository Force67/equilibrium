// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Handlers.h"
#include "flatbuffers/flatbuffers.h"
#include "net/NetClient.h"

#include "IdaInc.h"
#include <name.hpp>

#include "sync/SyncController.h"

namespace noda::sync::NameAddr
{
  using namespace protocol::sync;

  // raw pointer msg cast?
  bool Apply(SyncController &, const NameEa &pack)
  {
	return set_name(
	    static_cast<ea_t>(pack.ea()),
	    pack.name()->c_str(),
	    (pack.local() ? SN_LOCAL : 0) | SN_NOWARN);

	//msg("%x was named %s\n", pack->addr(), pack->name()->c_str());
  }

  bool React(SyncController &sc, va_list list)
  {
	auto addr = static_cast<uint64_t>(__crt_va_arg(list, ea_t));
	auto *name = va_arg(list, const char *);
	auto local = va_arg(list, int) != 0;

	net::FbsBuilder fbb;
	auto pack = CreateNameEa(fbb,
	                         addr,
	                         fbb.CreateString(name),
	                         static_cast<bool>(local));

	return sc.SendFbsPacket(fbb, protocol::MsgType_sync_NameEa, pack);
  }
} // namespace noda::sync::NameAddr