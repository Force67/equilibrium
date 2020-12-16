// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync/TaskHandler.h"

#include "Pch.h"
#include "enum.hpp"

#if 0

using namespace protocol::sync;

namespace noda::sync::create_enum {
  static bool Apply(SyncController &, const CreateEnum &pack)
  {
	LOG_TRACE("Created Enum {}", pack.name()->c_str());

	return add_enum(pack.id(), pack.name()->c_str(), 0) != BADADDR;
  }

  static bool React(SyncController &sc, va_list list)
  {
	enum_t eid = va_arg(list, enum_t);

	qstring name;
	const auto size = get_enum_name(&name, eid);
	if(size == 0)
	  return false;

	uval_t index = get_enum_idx(eid);

	auto pack = CreateCreateEnumDirect(sc.fbb(), index, name.c_str());
	return sc.SendFbsPacket(protocol::MsgType_sync_CreateEnum, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::enum_created,
	protocol::MsgType_sync_CreateEnum,
	SyncHandler::Handlers<CreateEnum>{ Apply, React }
  };
} // namespace noda::sync::create_enum

namespace noda::sync::delete_enum {
  static bool Apply(SyncController &, const DeleteEnum &pack)
  {
	enum_t eid = get_enum(pack.name()->c_str());
	if(eid == BADADDR)
	  return false;

	LOG_TRACE("Deleted Enum {}", pack.name()->c_str());
	del_enum(eid);
	return true;
  }

  static bool React(SyncController &sc, va_list list)
  {
	enum_t eid = va_arg(list, enum_t);

	qstring name;
	const auto size = get_enum_name(&name, eid);
	if(size == 0)
	  return false;

	auto pack = CreateDeleteEnumDirect(sc.fbb(), name.c_str());
	return sc.SendFbsPacket(protocol::MsgType_sync_DeleteEnum, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::deleting_enum,
	protocol::MsgType_sync_DeleteEnum,
	SyncHandler::Handlers<DeleteEnum>{ Apply, React }
  };
} // namespace noda::sync::delete_enum

#endif