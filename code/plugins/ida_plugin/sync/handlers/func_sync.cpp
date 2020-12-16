// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "sync/sync_handler.h"

using namespace protocol::sync;

namespace sync::add_function {
static bool Apply(SyncService&, const AddFunction& pack) {
  LOG_TRACE("Added function at {} with length {}", pack.startea(),
            pack.endea() - pack.startea());

  return add_func(pack.startea(), pack.endea());
}

static bool React(SyncService& sc, va_list list) {
  func_t* func = va_arg(list, func_t*);
  if (!func)
    return false;

  network::FbsBuffer buf;
  auto pack = CreateAddFunction(buf, func->start_ea, func->end_ea);

  return GNetClient()->SendPacket(protocol::MsgType_sync_AddFunction, buf,
                                  pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::func_added,
    protocol::MsgType_sync_AddFunction,
    StaticHandler::Handlers<AddFunction>{Apply, React}};
}  // namespace sync::add_function

namespace sync::delete_function {
static bool Apply(SyncService&, const DeleteFunction& pack) {
  LOG_TRACE("Deleted function at {}", pack.ea());
  return del_func(pack.ea());
}

static bool React(SyncService& sc, va_list list) {
  func_t* func = va_arg(list, func_t*);
  if (!func)
    return false;

  network::FbsBuffer buf;
  auto pack = CreateDeleteFunction(buf, func->start_ea);

  return GNetClient()->SendPacket(protocol::MsgType_sync_DeleteFunction, buf,
                                  pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::deleting_func,
    protocol::MsgType_sync_DeleteFunction,
    StaticHandler::Handlers<DeleteFunction>{Apply, React}};
}  // namespace sync::delete_function

namespace sync::set_function_start {
static bool Apply(SyncService&, const SetFunctionStart& pack) {
  LOG_TRACE("Changed function start {}", pack.ea());

  return set_func_start(pack.ea(), pack.newea());
}

static bool React(SyncService& sc, va_list list) {
  func_t* func = va_arg(list, func_t*);
  if (!func)
    return false;

  ea_t newStart = va_arg(list, ea_t);

  network::FbsBuffer buf;
  auto pack = CreateSetFunctionStart(buf, func->start_ea, newStart);

  return GNetClient()->SendPacket(protocol::MsgType_sync_SetFunctionStart, buf,
                                  pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::set_func_start,
    protocol::MsgType_sync_SetFunctionStart,
    StaticHandler::Handlers<SetFunctionStart>{Apply, React}};
}  // namespace sync::set_function_start

namespace sync::set_function_end {
static bool Apply(SyncService&, const SetFunctionEnd& pack) {
  LOG_TRACE("Changed function end {}", pack.startea());
  return set_func_end(pack.startea(), pack.endea());
}

static bool React(SyncService& sc, va_list list) {
  func_t* func = va_arg(list, func_t*);
  if (!func)
    return false;

  ea_t newEnd = va_arg(list, ea_t);

  network::FbsBuffer buf;
  auto pack = CreateSetFunctionEnd(buf, func->start_ea, newEnd);

  return GNetClient()->SendPacket(protocol::MsgType_sync_SetFunctionEnd, buf,
                                  pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::set_func_end,
    protocol::MsgType_sync_SetFunctionEnd,
    StaticHandler::Handlers<SetFunctionEnd>{Apply, React}};
}  // namespace sync::set_function_end

namespace sync::append_function_tail {
static bool Apply(SyncService&, const AppendFunctionTail& pack) {
  func_t* func = get_func(pack.funcea());
  if (!func)
    return false;

  LOG_TRACE("Appended function tail {}", pack.funcea());
  return append_func_tail(func, pack.tailEaStart(), pack.tailEaEnd());
}

static bool React(SyncService& sc, va_list list) {
  func_t* func = va_arg(list, func_t*);
  func_t* tail = va_arg(list, func_t*);
  if (!func || !tail)
    return false;

  network::FbsBuffer buf;
  auto pack = CreateAppendFunctionTail(buf, func->start_ea, tail->start_ea,
                                       tail->end_ea);

  return GNetClient()->SendPacket(protocol::MsgType_sync_AppendFunctionTail,
                                  buf, pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::func_tail_appended,
    protocol::MsgType_sync_AppendFunctionTail,
    StaticHandler::Handlers<AppendFunctionTail>{Apply, React}};
}  // namespace sync::append_function_tail

namespace sync::delete_function_tail {
static bool Apply(SyncService&, const DeleteFunctionTail& pack) {
  func_t* func = get_func(pack.funcea());
  if (!func)
    return false;

  LOG_TRACE("Deleted function tail {}", pack.funcea());
  return remove_func_tail(func, pack.tailea());
}

static bool React(SyncService& sc, va_list list) {
  func_t* func = va_arg(list, func_t*);
  ea_t tailEa = va_arg(list, ea_t);
  if (!func)
    return false;

  network::FbsBuffer buf;
  auto pack = CreateDeleteFunctionTail(buf, func->start_ea, tailEa);

  return GNetClient()->SendPacket(protocol::MsgType_sync_DeleteFunctionTail,
                                  buf, pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::func_tail_deleted,
    protocol::MsgType_sync_DeleteFunctionTail,
    StaticHandler::Handlers<DeleteFunctionTail>{Apply, React}};
}  // namespace sync::delete_function_tail

namespace sync::change_function_tail_owner {
static bool Apply(SyncService&, const ChangeFunctionTailOwner& pack) {
  func_t* tail = get_fchunk(pack.tailea());
  if (!tail)
    return false;

  LOG_TRACE("Changed function tail {}", pack.tailea());
  return set_tail_owner(tail, pack.funcstart());
}

static bool React(SyncService& sc, va_list list) {
  func_t* tail = va_arg(list, func_t*);
  ea_t ownerFunc = va_arg(list, ea_t);
  ea_t oldOwner = va_arg(list, ea_t);
  if (!tail)
    return false;

  network::FbsBuffer buf;
  auto pack = CreateChangeFunctionTailOwner(buf, tail->start_ea, ownerFunc);

  return GNetClient()->SendPacket(
      protocol::MsgType_sync_ChangeFunctionTailOwner, buf, pack.Union());
}

static StaticHandler handler{
    hook_type_t::HT_IDB, idb_event::tail_owner_changed,
    protocol::MsgType_sync_ChangeFunctionTailOwner,
    StaticHandler::Handlers<ChangeFunctionTailOwner>{Apply, React}};
}  // namespace sync::change_function_tail_owner