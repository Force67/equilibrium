// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "enum.hpp"

#include "sync/sync_handler.h"

using namespace protocol::sync;

namespace sync::create_enum {
static bool Apply(SyncService&, const CreateEnum& pack) {
  LOG_TRACE("Created Enum {}", pack.name()->c_str());

  return add_enum(pack.id(), pack.name()->c_str(), 0) != BADADDR;
}

static bool React(SyncService& sc, va_list list) {
  enum_t eid = va_arg(list, enum_t);

  qstring name;
  const auto size = get_enum_name(&name, eid);
  if (size == 0)
    return false;

  uval_t index = get_enum_idx(eid);

  network::FbsBuffer buf;
  auto pack = CreateCreateEnumDirect(buf, index, name.c_str());

  return GNetClient()->SendPacket(protocol::MsgType_sync_CreateEnum, buf,
                                  pack.Union());
}

static StaticHandler handler{hook_type_t::HT_IDB, idb_event::enum_created,
                             protocol::MsgType_sync_CreateEnum,
                             StaticHandler::Handlers<CreateEnum>{Apply, React}};
}  // namespace sync::create_enum

namespace sync::delete_enum {
static bool Apply(SyncService&, const DeleteEnum& pack) {
  enum_t eid = get_enum(pack.name()->c_str());
  if (eid == BADADDR)
    return false;

  LOG_TRACE("Deleted Enum {}", pack.name()->c_str());
  del_enum(eid);
  return true;
}

static bool React(SyncService& sc, va_list list) {
  enum_t eid = va_arg(list, enum_t);

  qstring name;
  const auto size = get_enum_name(&name, eid);
  if (size == 0)
    return false;

  network::FbsBuffer buf;
  auto pack = CreateDeleteEnumDirect(buf, name.c_str());

  return GNetClient()->SendPacket(protocol::MsgType_sync_DeleteEnum, buf,
                                  pack.Union());
}

static StaticHandler handler{hook_type_t::HT_IDB, idb_event::deleting_enum,
                             protocol::MsgType_sync_DeleteEnum,
                             StaticHandler::Handlers<DeleteEnum>{Apply, React}};
}  // namespace sync::delete_enum
