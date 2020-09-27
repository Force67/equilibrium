// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncHandler.h"
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

		auto pack = CreateNameEaDirect(sc.fbb(), addr, name,
		                               static_cast<bool>(local));

		return sc.SendFbsPacket(protocol::MsgType_sync_NameEa, pack);
	}

	static SyncHandler handler_registry{
		hook_type_t::HT_IDB,
		idb_event::renamed,
		protocol::MsgType_sync_NameEa,
		SyncHandler::Delegates<NameEa>{ Apply, React }
	};
} // namespace noda::sync::NameAddr