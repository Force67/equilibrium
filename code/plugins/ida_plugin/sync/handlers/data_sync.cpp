// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "sync/sync_handler.h"

using namespace protocol::sync;

namespace sync::make_code {
static bool Apply(SyncService&, const MakeCode& pack) {
  LOG_TRACE("MakeCode {:x}", pack.ea());
  return create_insn(pack.ea());
}

static bool React(SyncService& sc, va_list list) {
  const insn_t* insn = va_arg(list, ::insn_t*);
  if (!insn)
    return false;

  network::FbsBuffer buf;
  auto pack = CreateMakeCode(buf, insn->ea);

  return GNetClient()->SendPacket(protocol::MsgType_sync_MakeCode, buf,
                                  pack.Union());
}

static StaticHandler handler{hook_type_t::HT_IDB, idb_event::make_code,
                             protocol::MsgType_sync_MakeCode,
                             StaticHandler::Handlers<MakeCode>{Apply, React}};
}  // namespace sync::make_code

namespace sync::make_data {
static bool Apply(SyncService&, const MakeData& pack) {
  LOG_TRACE("MakeData {:x}", pack.ea());

  return create_data(pack.ea(), pack.flags(),
                     static_cast<asize_t>(pack.length()), pack.tid());
}

static bool React(SyncService& sc, va_list list) {
  ea_t ea = va_arg(list, ea_t);
  auto flags = va_arg(list, ::flags_t);
  auto tid = va_arg(list, ::tid_t);
  auto len = va_arg(list, ::asize_t);

  network::FbsBuffer buf;
  auto pack = CreateMakeData(buf, ea, flags, static_cast<uint32_t>(len), tid);

  return GNetClient()->SendPacket(protocol::MsgType_sync_MakeData, buf,
                                  pack.Union());
}

static StaticHandler handler{hook_type_t::HT_IDB, idb_event::make_data,
                             protocol::MsgType_sync_MakeData,
                             StaticHandler::Handlers<MakeData>{Apply, React}};
}  // namespace sync::make_data

namespace sync::name_address {
using namespace protocol::sync;

static bool Apply(SyncService&, const NameEa& pack) {
  LOG_TRACE("{:x} was named {}", pack.ea(), pack.name()->c_str());

  bool result = set_name(static_cast<ea_t>(pack.ea()), pack.name()->c_str(),
                         (pack.local() ? SN_LOCAL : 0) | SN_NOWARN);

  if (result) {
    request_refresh(IWID_DISASMS);
    return true;
  }

  return false;
}

static bool React(SyncService& sc, va_list list) {
  auto addr = static_cast<uint64_t>(__crt_va_arg(list, ea_t));
  auto* name = va_arg(list, const char*);
  auto local = va_arg(list, int) != 0;

  network::FbsBuffer buf;
  auto pack = CreateNameEaDirect(buf, addr, name, static_cast<bool>(local));

  return GNetClient()->SendPacket(protocol::MsgType_sync_NameEa, buf,
                                  pack.Union());
}

static StaticHandler handler{hook_type_t::HT_IDB, idb_event::renamed,
                             protocol::MsgType_sync_NameEa,
                             StaticHandler::Handlers<NameEa>{Apply, React}};
}  // namespace sync::name_address