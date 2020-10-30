// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#pragma once

#include <ida.hpp>

#include "sync/NSyncController.h"
#include "moc_protocol/Message_generated.h"
#include "utils/Logger.h"

namespace noda {
  using namespace protocol::sync;

  struct SyncHandler;

  struct SyncHandlerRegistry {
	SyncHandlerRegistry() = default;
	~SyncHandlerRegistry() = default;

	SyncHandlerRegistry(SyncHandler *hndlr) :
	    next(ROOT()),
	    handler(hndlr)
	{
	  ROOT() = this;
	}

	SyncHandlerRegistry *next = nullptr;
	SyncHandler *handler = nullptr;

	static SyncHandlerRegistry *&ROOT()
	{
	  static SyncHandlerRegistry *root{ nullptr };
	  return root;
	}
  };

  struct SyncHandler : SyncHandlerRegistry {
	// IDA Event
	hook_type_t hookType = hook_type_t::HT_LAST;
	int hookEvent = -1;

	// Networking
	protocol::MsgType msgType = protocol::MsgType_NONE;

	using apply_t = bool (*)(NSyncController &, const void *);
	using react_t = bool (*)(NSyncController &, va_list);

	// Delegate
	struct BaseHandler {
	  apply_t apply;
	  react_t react;
	};

	BaseHandler delegates;

	template <typename T>
	struct Handlers : BaseHandler {
	  using apply_t = bool (*)(NSyncController &, const T &);

	  explicit Handlers(apply_t apply_impl, react_t react_impl)
	  {
		apply = reinterpret_cast<SyncHandler::apply_t>(reinterpret_cast<void *>(apply_impl));
		react = react_impl;
	  }
	};

	explicit SyncHandler(hook_type_t type, int evt, protocol::MsgType msg, BaseHandler &&methods) :
	    hookType(type),
	    hookEvent(evt),
	    msgType(msg),
	    delegates(std::forward<BaseHandler>(methods)),
	    SyncHandlerRegistry(this)
	{
	}

	~SyncHandler() = default;
  };
} // namespace noda