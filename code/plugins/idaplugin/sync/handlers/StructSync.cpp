// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync/SyncHandler.h"

#include "IdaInc.h"
#include "struct.hpp"

#if 0

using namespace protocol::sync;

namespace noda::sync::create_struct {
  static bool Apply(SyncController &, const CreateStruct &pack)
  {
	LOG_TRACE("Created struct {}", pack.name()->c_str());

	return add_struc(BADADDR, pack.name()->c_str(), pack.isUnion()) != BADADDR;
  }

  static bool React(SyncController &sc, va_list list)
  {
	tid_t sid = va_arg(list, tid_t);
	struc_t *struc = get_struc(sid);

	auto pack = CreateCreateStructDirect(sc.fbb(),
	                                     get_struc_name(sid).c_str(),
	                                     struc->is_union());

	return sc.SendFbsPacket(protocol::MsgType_sync_CreateStruct, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::struc_created,
	protocol::MsgType_sync_CreateStruct,
	SyncHandler::Handlers<CreateStruct>{ Apply, React }
  };
} // namespace noda::sync::create_struct

namespace noda::sync::delete_struct {
  static bool Apply(SyncController &, const DeleteStruct &pack)
  {
	LOG_TRACE("Struct {} was deleted ", pack.name()->c_str());

	tid_t sid = get_struc_id(pack.name()->c_str());
	if(sid != BADADDR) {
	  del_struc(get_struc(sid));
	  return true;
	}

	return false;
  }

  static bool React(SyncController &sc, va_list list)
  {
	struc_t *struc = va_arg(list, struc_t *);
	auto name = get_struc_name(struc->id);

	auto pack = CreateDeleteStructDirect(sc.fbb(), name.c_str());
	return sc.SendFbsPacket(protocol::MsgType_sync_DeleteStruct, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::deleting_struc,
	protocol::MsgType_sync_DeleteStruct,
	SyncHandler::Handlers<DeleteStruct>{ Apply, React }
  };
} // namespace noda::sync::delete_struct

namespace noda::sync::rename_struct {
  static bool Apply(SyncController &, const RenameStruct &pack)
  {
	LOG_TRACE("Renamed Struct {} to {}",
	          pack.oldName()->c_str(), pack.newName()->c_str());

	tid_t sid = get_struc_id(pack.oldName()->c_str());
	if(sid == BADADDR)
	  return false;

	return set_struc_name(sid, pack.newName()->c_str());
  }

  static bool React(SyncController &sc, va_list list)
  {
	tid_t sid = va_arg(list, tid_t);
	auto *oldName = va_arg(list, const char *);
	auto *newName = va_arg(list, const char *);

	auto pack = CreateRenameStructDirect(sc.fbb(), oldName, newName);
	return sc.SendFbsPacket(protocol::MsgType_sync_RenameStruct, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::renaming_struc,
	protocol::MsgType_sync_RenameStruct,
	SyncHandler::Handlers<RenameStruct>{ Apply, React }
  };
} // namespace noda::sync::rename_struct

namespace noda::sync::rename_struct_member {
  static bool Apply(SyncController &, const RenameStructMember &pack)
  {
	tid_t sid = get_struc_id(pack.structName()->c_str());
	if(sid == BADADDR)
	  return false;

	return set_member_name(get_struc(sid), pack.offset(), pack.memberName()->c_str());
  }

  static bool React(SyncController &sc, va_list list)
  {
	struc_t *struc = va_arg(list, struc_t *);
	member_t *oldName = va_arg(list, member_t *);
	auto *newName = va_arg(list, const char *);

	auto name = get_struc_name(struc->id);

	auto pack = CreateRenameStructMemberDirect(sc.fbb(), name.c_str(), newName);
	return sc.SendFbsPacket(protocol::MsgType_sync_RenameStructMember, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::renaming_struc_member,
	protocol::MsgType_sync_RenameStructMember,
	SyncHandler::Handlers<RenameStructMember>{ Apply, React }
  };
} // namespace noda::sync::rename_struct_member

namespace noda::sync::delete_struct_member {
  static bool Apply(SyncController &, const DeleteStructMember &pack)
  {
	tid_t sid = get_struc_id(pack.name()->c_str());
	if(sid == BADADDR)
	  return false;

	LOG_TRACE("Deleting member {} of struct {}", pack.ea(), pack.name()->c_str());
	return del_struc_member(get_struc(sid), pack.ea());
  }

  static bool React(SyncController &sc, va_list list)
  {
	struc_t *struc = va_arg(list, struc_t *);
	tid_t memberId = va_arg(list, tid_t);
	ea_t offset = va_arg(list, ea_t);

	auto name = get_struc_name(struc->id);

	auto pack = CreateDeleteStructMemberDirect(sc.fbb(), offset, name.c_str());
	return sc.SendFbsPacket(protocol::MsgType_sync_DeleteStructMember, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::struc_member_deleted,
	protocol::MsgType_sync_DeleteStructMember,
	SyncHandler::Handlers<DeleteStructMember>{ Apply, React }
  };
} // namespace noda::sync::delete_struct_member

namespace noda::sync::change_struct_member {
  static bool Apply(SyncController &, const ChangeStructMember &pack)
  {
	tid_t sid = get_struc_id(pack.structName()->c_str());
	if(sid == BADADDR)
	  return false;

	struc_t *struc = get_struc(sid);

	switch(pack.type()) {
	case StructMemberType_Struct:
	  break;
	case StructMemberType_String:
	  break;
	case StructMemberType_Offset:
	  break;
	}

	LOG_ERROR("TODO: ChangeStructMember");
	return true;
  }

  static bool React(SyncController &sc, va_list list)
  {
	struc_t *struc = va_arg(list, ::struc_t *);
	member_t *member = va_arg(list, ::member_t *);

	if(!struc || !member)
	  return false;

	const auto strucName = get_struc_name(struc->id);
	const auto memberName = get_member_name(member->id);

	//auto pack = CreateChangeStructMemberDirect(sc.fbb(),  strucName.c_str(), memberName.c_str(),

	LOG_ERROR("TODO: Send ChangeStructMember");
	return true;
  }

  static SyncHandler handler_create{
	hook_type_t::HT_IDB,
	idb_event::struc_member_created,
	protocol::MsgType_sync_ChangeStructMember,
	SyncHandler::Handlers<ChangeStructMember>{ Apply, React }
  };

  static SyncHandler handler_change{
	hook_type_t::HT_IDB,
	idb_event::struc_member_changed,
	protocol::MsgType_sync_ChangeStructMember,
	SyncHandler::Handlers<ChangeStructMember>{ Apply, React }
  };
} // namespace noda::sync::change_struct_member

#endif