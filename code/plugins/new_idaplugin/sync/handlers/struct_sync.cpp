// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "sync/SyncHandler.h"

using namespace protocol::sync;

namespace sync::create_struct {
static bool Apply(SyncService&, const CreateStruct& pack) {
  LOG_TRACE("Created struct {}", pack.name()->c_str());

  return add_struc(BADADDR, pack.name()->c_str(), pack.isUnion()) != BADADDR;
}

static bool React(SyncService& sc, va_list list) {
  tid_t sid = va_arg(list, tid_t);
  struc_t* struc = get_struc(sid);

  network::FbsBuffer buf;
  auto pack = CreateCreateStructDirect(buf, get_struc_name(sid).c_str(),
                                       struc->is_union());

  return GNetClient()->SendPacket(protocol::MsgType_sync_CreateStruct, buf,
                                  pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::struc_created,
    protocol::MsgType_sync_CreateStruct,
    StaticHandler::Handlers<CreateStruct>{Apply, React}};
}  // namespace sync::create_struct

namespace sync::delete_struct {
static bool Apply(SyncService&, const DeleteStruct& pack) {
  LOG_TRACE("Struct {} was deleted ", pack.name()->c_str());

  tid_t sid = get_struc_id(pack.name()->c_str());
  if (sid != BADADDR) {
    del_struc(get_struc(sid));
    return true;
  }

  return false;
}

static bool React(SyncService& sc, va_list list) {
  struc_t* struc = va_arg(list, struc_t*);
  auto name = get_struc_name(struc->id);

  network::FbsBuffer buf;
  auto pack = CreateDeleteStructDirect(buf, name.c_str());

  return GNetClient()->SendPacket(protocol::MsgType_sync_DeleteStruct, buf,
                                  pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::deleting_struc,
    protocol::MsgType_sync_DeleteStruct,
    StaticHandler::Handlers<DeleteStruct>{Apply, React}};
}  // namespace sync::delete_struct

namespace sync::rename_struct {
static bool Apply(SyncService&, const RenameStruct& pack) {
  LOG_TRACE("Renamed Struct {} to {}", pack.oldName()->c_str(),
            pack.newName()->c_str());

  tid_t sid = get_struc_id(pack.oldName()->c_str());
  if (sid == BADADDR)
    return false;

  return set_struc_name(sid, pack.newName()->c_str());
}

static bool React(SyncService& sc, va_list list) {
  tid_t sid = va_arg(list, tid_t);
  auto* oldName = va_arg(list, const char*);
  auto* newName = va_arg(list, const char*);

  network::FbsBuffer buf;
  auto pack = CreateRenameStructDirect(buf, oldName, newName);

  return GNetClient()->SendPacket(protocol::MsgType_sync_RenameStruct, buf,
                                  pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::renaming_struc,
    protocol::MsgType_sync_RenameStruct,
    StaticHandler::Handlers<RenameStruct>{Apply, React}};
}  // namespace sync::rename_struct

namespace sync::rename_struct_member {
static bool Apply(SyncService&, const RenameStructMember& pack) {
  tid_t sid = get_struc_id(pack.structName()->c_str());
  if (sid == BADADDR)
    return false;

  return set_member_name(get_struc(sid), pack.offset(),
                         pack.memberName()->c_str());
}

static bool React(SyncService& sc, va_list list) {
  struc_t* struc = va_arg(list, struc_t*);
  member_t* oldName = va_arg(list, member_t*);
  auto* newName = va_arg(list, const char*);

  auto name = get_struc_name(struc->id);

  network::FbsBuffer buf;
  auto pack = CreateRenameStructMemberDirect(buf, name.c_str(), newName);

  return GNetClient()->SendPacket(protocol::MsgType_sync_RenameStructMember,
                                  buf, pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::renaming_struc_member,
    protocol::MsgType_sync_RenameStructMember,
    StaticHandler::Handlers<RenameStructMember>{Apply, React}};
}  // namespace sync::rename_struct_member

namespace sync::delete_struct_member {
static bool Apply(SyncService&, const DeleteStructMember& pack) {
  tid_t sid = get_struc_id(pack.name()->c_str());
  if (sid == BADADDR)
    return false;

  LOG_TRACE("Deleting member {} of struct {}", pack.ea(), pack.name()->c_str());
  return del_struc_member(get_struc(sid), pack.ea());
}

static bool React(SyncService& sc, va_list list) {
  struc_t* struc = va_arg(list, struc_t*);
  tid_t memberId = va_arg(list, tid_t);
  ea_t offset = va_arg(list, ea_t);

  auto name = get_struc_name(struc->id);

  network::FbsBuffer buf;
  auto pack = CreateDeleteStructMemberDirect(buf, offset, name.c_str());

  return GNetClient()->SendPacket(protocol::MsgType_sync_DeleteStructMember,
                                  buf, pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::struc_member_deleted,
    protocol::MsgType_sync_DeleteStructMember,
    StaticHandler::Handlers<DeleteStructMember>{Apply, React}};
}  // namespace sync::delete_struct_member

namespace sync::change_struct_member {
static bool Apply(SyncService&, const ChangeStructMember& pack) {
  tid_t sid = get_struc_id(pack.structName()->c_str());
  if (sid == BADADDR)
    return false;

  struc_t* struc = get_struc(sid);

  switch (pack.type()) {
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

static bool React(SyncService& sc, va_list list) {
  struc_t* struc = va_arg(list, ::struc_t*);
  member_t* member = va_arg(list, ::member_t*);

  if (!struc || !member)
    return false;

  const auto strucName = get_struc_name(struc->id);
  const auto memberName = get_member_name(member->id);

  // auto pack = CreateChangeStructMemberDirect(sc.fbb(),  strucName.c_str(),
  // memberName.c_str(),

  LOG_ERROR("TODO: Send ChangeStructMember");
  return true;
}

static StaticHandler handler_create{
    hook_type_t::HT_IDB, idb_event::struc_member_created,
    protocol::MsgType_sync_ChangeStructMember,
    StaticHandler::Handlers<ChangeStructMember>{Apply, React}};

static StaticHandler handler_change{
    hook_type_t::HT_IDB, idb_event::struc_member_changed,
    protocol::MsgType_sync_ChangeStructMember,
    StaticHandler::Handlers<ChangeStructMember>{Apply, React}};
}  // namespace sync::change_struct_member