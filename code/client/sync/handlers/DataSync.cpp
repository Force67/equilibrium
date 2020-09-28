// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncHandler.h"

#include "IdaInc.h"

#include "name.hpp"
#include "enum.hpp"

using namespace protocol::sync;

namespace noda::sync::make_code
{
	static bool Apply(SyncController&, const MakeCode& pack)
	{
		LOG_TRACE("MakeCode {:x}", pack.ea());
		return create_insn(pack.ea());
	}

	static bool React(SyncController& sc, va_list list)
	{
		const insn_t* insn = va_arg(list, ::insn_t*);
		if (!insn)
			return false;

		auto pack = CreateMakeCode(sc.fbb(), insn->ea);
		return sc.SendFbsPacket(protocol::MsgType_sync_MakeCode, pack);
	}

	static SyncHandler handler{
		hook_type_t::HT_IDB,
		idb_event::make_code ,
		protocol::MsgType_sync_MakeCode,
		SyncHandler::Handlers<MakeCode>{ Apply, React }
	};
} // namespace noda::sync::make_code

namespace noda::sync::make_data
{
	static bool Apply(SyncController&, const MakeData& pack)
	{
		LOG_TRACE("MakeData {:x}", pack.ea());

		return create_data(pack.ea(), pack.flags(), 
			static_cast<asize_t>(pack.length()), pack.tid());
	}

	static bool React(SyncController& sc, va_list list)
	{
		ea_t ea = va_arg(list, ea_t);
		auto flags = va_arg(list, ::flags_t);
		auto tid = va_arg(list, ::tid_t);
		auto len = va_arg(list, ::asize_t);

		auto pack = CreateMakeData(sc.fbb(), ea, flags, static_cast<uint32_t>(len), tid);
		return sc.SendFbsPacket(protocol::MsgType_sync_MakeData, pack);
	}

	static SyncHandler handler{
		hook_type_t::HT_IDB,
		idb_event::make_data ,
		protocol::MsgType_sync_MakeData,
		SyncHandler::Handlers<MakeData>{ Apply, React }
	};
} // namespace noda::sync::make_data

namespace noda::sync::name_address
{
	using namespace protocol::sync;

	static bool Apply(SyncController&, const NameEa& pack)
	{
		LOG_TRACE("{:x} was named {}", pack.ea(), pack.name()->c_str());

		bool result =  set_name(static_cast<ea_t>(pack.ea()), pack.name()->c_str(),
			(pack.local() ? SN_LOCAL : 0) | SN_NOWARN);

		if (result) {
			request_refresh(IWID_DISASMS);
			return true;
		}

		return false;
	}

	static bool React(SyncController& sc, va_list list)
	{
		auto addr = static_cast<uint64_t>(__crt_va_arg(list, ea_t));
		auto* name = va_arg(list, const char*);
		auto local = va_arg(list, int) != 0;

		auto pack = CreateNameEaDirect(sc.fbb(), addr, name,
			static_cast<bool>(local));

		return sc.SendFbsPacket(protocol::MsgType_sync_NameEa, pack);
	}

	static SyncHandler handler{
		hook_type_t::HT_IDB,
		idb_event::renamed,
		protocol::MsgType_sync_NameEa,
		SyncHandler::Handlers<NameEa>{ Apply, React }
	};
} // namespace noda::sync::name_address