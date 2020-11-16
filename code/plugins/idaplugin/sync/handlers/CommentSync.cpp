// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync/SyncHandler.h"

#include "Pch.h"
#include "struct.hpp"
#if 0
using namespace protocol::sync;

namespace noda::sync::change_item_comment {
  static bool Apply(SyncController &, const ChangeItemComment &pack)
  {
	LOG_TRACE("Changed Item comment {}", pack.comment()->c_str());
	return set_cmt(pack.ea(), pack.comment()->c_str(), pack.repeatable());
  }

  static bool React(SyncController &sc, va_list list)
  {
	ea_t ea = va_arg(list, ea_t);
	bool rpt = va_arg(list, bool);
	auto *newComment = va_arg(list, const char *);

	if(!newComment)
	  return false;

	auto pack = CreateChangeItemCommentDirect(sc.fbb(), ea, newComment, rpt);
	return sc.SendFbsPacket(protocol::MsgType_sync_ChangeItemComment, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::changing_cmt,
	protocol::MsgType_sync_ChangeItemComment,
	SyncHandler::Handlers<ChangeItemComment>{ Apply, React }
  };
} // namespace noda::sync::change_item_comment

namespace noda::sync::change_range_comment {
  static bool Apply(SyncController &, const ChangeRangeComment &pack)
  {
	if(pack.kind() == RANGE_KIND_FUNC) {
	  func_t *func = get_func(pack.ea());

	  return set_func_cmt(func,
	                      pack.comment()->c_str(), pack.repeatable());
	}
	else if(pack.kind() == RANGE_KIND_SEGMENT) {
	  auto *seg = getseg(pack.ea());

	  set_segment_cmt(seg,
	                  pack.comment()->c_str(), pack.repeatable());

	  return true;
	}

	LOG_WARNING("ChangeRangeComment: Unsupported kind {}", pack.kind());
	return false;
  }

  static bool React(SyncController &sc, va_list list)
  {
	auto kind = va_arg(list, ::range_kind_t);
	const range_t *range = va_arg(list, const range_t *);
	const char *comment = va_arg(list, const char *);
	bool rpt = va_arg(list, bool);

	if(!range)
	  return false;

	auto pack = CreateChangeRangeCommentDirect(sc.fbb(),
	                                           range->start_ea, kind, comment, rpt);

	return sc.SendFbsPacket(protocol::MsgType_sync_ChangeRangeComment, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::range_cmt_changed,
	protocol::MsgType_sync_ChangeRangeComment,
	SyncHandler::Handlers<ChangeRangeComment>{ Apply, React }
  };
} // namespace noda::sync::change_range_comment

namespace noda::sync::change_extra_comment {
  static bool Apply(SyncController &, const ChangeExtraComment &pack)
  {
	del_extra_cmt(pack.ea(), pack.line());

	if(!pack.comment()->c_str())
	  return false;

	const bool isPrev = pack.line() - 1000 < 1000;
	return add_extra_cmt(pack.ea(), isPrev, pack.comment()->c_str());
  }

  static bool React(SyncController &sc, va_list list)
  {
	ea_t ea = va_arg(list, ea_t);
	int line = va_arg(list, int);
	const char *comment = va_arg(list, const char *);

	if(!comment)
	  return false;

	auto pack = CreateChangeExtraCommentDirect(sc.fbb(), ea, line, comment);
	return sc.SendFbsPacket(protocol::MsgType_sync_ChangeExtraComment, pack);
  }

  static SyncHandler handler{
	hook_type_t::HT_IDB,
	idb_event::extra_cmt_changed,
	protocol::MsgType_sync_ChangeExtraComment,
	SyncHandler::Handlers<ChangeExtraComment>{ Apply, React }
  };
} // namespace noda::sync::change_extra_comment

#endif