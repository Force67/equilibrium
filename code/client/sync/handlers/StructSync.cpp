// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncHandler.h"

#include "IdaInc.h"
#include "struct.hpp"

namespace noda::sync::create_struct
{
	using namespace protocol::sync;

	static bool Apply(SyncController&, const CreateStruct& pack)
	{
		LOG_TRACE("Created struct {}", pack.name()->c_str());

		tid_t sid = add_struc(BADADDR, pack.name()->c_str(), pack.isUnion());

		LOG_ERROR("STORE STRUCT NAME???");

		return sid != BADADDR;
	}

	static bool React(SyncController& sc, va_list list)
	{
		tid_t sid = va_arg(list, tid_t);
		struc_t* struc = get_struc(sid);

		auto pack = CreateCreateStructDirect(sc.fbb(), get_struc_name(sid).c_str(), struc->is_union());

		// TODO: GET NAME

		LOG_ERROR("Unhandeled, create struct name!!!!");

#if 0
		auto pack = CreateDeleteStructDirect(sc.fbb(), );

		return sc.SendFbsPacket(protocol::MsgType_sync_NameEa, pack);
#endif
		return true;
	}

	static SyncHandler handler{
		hook_type_t::HT_IDB,
		idb_event::struc_created,
		protocol::MsgType_sync_CreateStruct,
		SyncHandler::Handlers<CreateStruct>{ Apply, React }
	};
} // namespace noda::sync::delete_struct

namespace noda::sync::delete_struct
{
	using namespace protocol::sync;

	static bool Apply(SyncController&, const DeleteStruct& pack)
	{
		LOG_TRACE("Struct {} was deleted ", pack.name()->c_str());

		tid_t sid = get_struc_id(pack.name()->c_str());
		if (sid != BADADDR) {
			del_struc(get_struc(sid));
			return true;
		}

		return false;
	}

	static bool React(SyncController& sc, va_list list)
	{
		tid_t sid = va_arg(list, tid_t);

		// TODO: GET NAME

		LOG_ERROR("Unhandeled, delete struct name!!!!");

#if 0
		auto pack = CreateDeleteStructDirect(sc.fbb(), );

		return sc.SendFbsPacket(protocol::MsgType_sync_NameEa, pack);
#endif
		return true;
	}

	static SyncHandler handler{
		hook_type_t::HT_IDB,
		idb_event::struc_deleted,
		protocol::MsgType_sync_DeleteStruct,
		SyncHandler::Handlers<DeleteStruct>{ Apply, React }
	};
} // namespace noda::sync::delete_struct

