// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync/SyncHandler.h"

#include "IdaInc.h"
#include "name.hpp"

#if 0

using namespace protocol::sync;

namespace noda::sync::add_function {
  static bool Apply(SyncController &, const AddFunction &pack)
  {
	LOG_TRACE("Added function at {} with length {}",
	          pack.startea(), pack.endea() - pack.startea());

	return add_func(pack.startea(), pack.endea());
  }

  static bool React(SyncController &sc, va_list list)
  {
	func_t *func = va_arg(list, func_t *);
	if(!func)
	  return false;

	auto pack = CreateAddFunction(sc.fbb(), func->start_ea, func->end_ea);
	return sc.SendFbsPacket(protocol::MsgType_sync_AddFunction, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::func_added,
	protocol::MsgType_sync_AddFunction,
	SyncHandler::Handlers<AddFunction>{ Apply, React }
  };
} // namespace noda::sync::add_function

namespace noda::sync::delete_function {
  static bool Apply(SyncController &, const DeleteFunction &pack)
  {
	LOG_TRACE("Deleted function at {}", pack.ea());
	return del_func(pack.ea());
  }

  static bool React(SyncController &sc, va_list list)
  {
	func_t *func = va_arg(list, func_t *);
	if(!func)
	  return false;

	auto pack = CreateDeleteFunction(sc.fbb(), func->start_ea);
	return sc.SendFbsPacket(protocol::MsgType_sync_DeleteFunction, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::deleting_func,
	protocol::MsgType_sync_DeleteFunction,
	SyncHandler::Handlers<DeleteFunction>{ Apply, React }
  };
} // namespace noda::sync::delete_function

namespace noda::sync::set_function_start {
  static bool Apply(SyncController &, const SetFunctionStart &pack)
  {
	LOG_TRACE("Changed function start {}", pack.ea());

	return set_func_start(pack.ea(), pack.newea());
  }

  static bool React(SyncController &sc, va_list list)
  {
	func_t *func = va_arg(list, func_t *);
	if(!func)
	  return false;

	ea_t newStart = va_arg(list, ea_t);

	auto pack = CreateSetFunctionStart(sc.fbb(), func->start_ea, newStart);
	return sc.SendFbsPacket(protocol::MsgType_sync_SetFunctionStart, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::set_func_start,
	protocol::MsgType_sync_SetFunctionStart,
	SyncHandler::Handlers<SetFunctionStart>{ Apply, React }
  };
} // namespace noda::sync::set_function_start

namespace noda::sync::set_function_end {
  static bool Apply(SyncController &, const SetFunctionEnd &pack)
  {
	LOG_TRACE("Changed function end {}", pack.startea());
	return set_func_end(pack.startea(), pack.endea());
  }

  static bool React(SyncController &sc, va_list list)
  {
	func_t *func = va_arg(list, func_t *);
	if(!func)
	  return false;

	ea_t newEnd = va_arg(list, ea_t);

	auto pack = CreateSetFunctionEnd(sc.fbb(), func->start_ea, newEnd);
	return sc.SendFbsPacket(protocol::MsgType_sync_SetFunctionEnd, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::set_func_end,
	protocol::MsgType_sync_SetFunctionEnd,
	SyncHandler::Handlers<SetFunctionEnd>{ Apply, React }
  };
} // namespace noda::sync::set_function_end

namespace noda::sync::append_function_tail {
  static bool Apply(SyncController &, const AppendFunctionTail &pack)
  {
	func_t *func = get_func(pack.funcea());
	if(!func)
	  return false;

	LOG_TRACE("Appended function tail {}", pack.funcea());
	return append_func_tail(func, pack.tailEaStart(), pack.tailEaEnd());
  }

  static bool React(SyncController &sc, va_list list)
  {
	func_t *func = va_arg(list, func_t *);
	func_t *tail = va_arg(list, func_t *);
	if(!func || !tail)
	  return false;

	auto pack = CreateAppendFunctionTail(sc.fbb(),
	                                     func->start_ea, tail->start_ea, tail->end_ea);

	return sc.SendFbsPacket(protocol::MsgType_sync_AppendFunctionTail, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::func_tail_appended,
	protocol::MsgType_sync_AppendFunctionTail,
	SyncHandler::Handlers<AppendFunctionTail>{ Apply, React }
  };
} // namespace noda::sync::append_function_tail

namespace noda::sync::delete_function_tail {
  static bool Apply(SyncController &, const DeleteFunctionTail &pack)
  {
	func_t *func = get_func(pack.funcea());
	if(!func)
	  return false;

	LOG_TRACE("Deleted function tail {}", pack.funcea());
	return remove_func_tail(func, pack.tailea());
  }

  static bool React(SyncController &sc, va_list list)
  {
	func_t *func = va_arg(list, func_t *);
	ea_t tailEa = va_arg(list, ea_t);
	if(!func)
	  return false;

	auto pack = CreateDeleteFunctionTail(sc.fbb(),
	                                     func->start_ea, tailEa);

	return sc.SendFbsPacket(protocol::MsgType_sync_DeleteFunctionTail, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::func_tail_deleted,
	protocol::MsgType_sync_DeleteFunctionTail,
	SyncHandler::Handlers<DeleteFunctionTail>{ Apply, React }
  };
} // namespace noda::sync::delete_function_tail

namespace noda::sync::change_function_tail_owner {
  static bool Apply(SyncController &, const ChangeFunctionTailOwner &pack)
  {
	func_t *tail = get_fchunk(pack.tailea());
	if(!tail)
	  return false;

	LOG_TRACE("Changed function tail {}", pack.tailea());
	return set_tail_owner(tail, pack.funcstart());
  }

  static bool React(SyncController &sc, va_list list)
  {
	func_t *tail = va_arg(list, func_t *);
	ea_t ownerFunc = va_arg(list, ea_t);
	ea_t oldOwner = va_arg(list, ea_t);
	if(!tail)
	  return false;

	auto pack = CreateChangeFunctionTailOwner(sc.fbb(),
	                                          tail->start_ea, ownerFunc);

	return sc.SendFbsPacket(protocol::MsgType_sync_ChangeFunctionTailOwner, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::tail_owner_changed,
	protocol::MsgType_sync_ChangeFunctionTailOwner,
	SyncHandler::Handlers<ChangeFunctionTailOwner>{ Apply, React }
  };
} // namespace noda::sync::change_function_tail_owner

#endif