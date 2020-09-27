// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#pragma once
#include "net/protocol/Message_generated.h"
#include <ida.hpp>

namespace noda::sync
{
	using namespace protocol::sync;

	class SyncController;
	struct SyncHandler;

	namespace detail
	{
		struct SyncHandler_InitDelegate {
			SyncHandler_InitDelegate() = default;
			~SyncHandler_InitDelegate() = default;

			SyncHandler_InitDelegate(SyncHandler *hndlr) :
			    next(ROOT()),
			    handler(hndlr)
			{
				ROOT()->next = this;
			}

			SyncHandler_InitDelegate *next = nullptr;
			SyncHandler *handler = nullptr;

			static SyncHandler_InitDelegate *ROOT()
			{
				static SyncHandler_InitDelegate root;
				return &root;
			}
		};
	} // namespace detail

	struct SyncHandler : detail::SyncHandler_InitDelegate {
		// IDA Event
		hook_type_t hookType = hook_type_t::HT_LAST;
		int hookEvent = -1;

		// Networking
		protocol::MsgType msgType = protocol::MsgType_NONE;

		// Delegate
		struct BaseDelegate {
			using apply_t = bool (*)(SyncController &, const void *);
			apply_t apply;

			using react_t = bool (*)(SyncController &, va_list);
			react_t react;
		};

		BaseDelegate delegates;

		template <typename T>
		struct Delegates : BaseDelegate {
			using apply_t = bool (*)(SyncController &, const T &);

			Delegates(apply_t apply_impl, react_t react_impl)
			{
				apply = static_cast<BaseDelegate::apply_t>(static_cast<void *>(apply_impl));
				react = react_impl;
			}
		};

		SyncHandler(hook_type_t type, int evt, protocol::MsgType msg, BaseDelegate &&methods) :
		    hookType(type),
		    hookEvent(evt),
		    msgType(msg),
		    delegates(std::forward<BaseDelegate>(methods)),
		    SyncHandler_InitDelegate(this)
		{
		}

		~SyncHandler() = default;
	};
} // namespace noda::sync