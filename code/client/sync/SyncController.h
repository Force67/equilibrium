// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <qobject.h>
#include "net/NetBase.h"
#include "utils/Storage.h"
#include "utils/Logger.h"
#include "net/protocol/Message_generated.h"
#include <map>

namespace QT
{
	class QTimer;
}

namespace noda
{
	namespace net
	{
		class NetClient;
	}

	namespace sync
	{
		struct SyncHandler;

		class SyncController final : public QObject,
		                             public net::NetDelegate
		{
			Q_OBJECT;

		  public:
			SyncController();
			SyncController(const SyncController &) = delete;
			~SyncController();

			bool ConnectServer();
			void DisconnectServer();

			bool IsConnected();

			// type safe wrapper to guarntee proper specifization
			template <typename T>
			inline bool SendFbsPacket(
			    protocol::MsgType tt,
			    const net::FbsOffset<T> ref)
			{
				return _client->SendFbsPacketReliable(_fbb, tt, ref.Union());
			}

			// Get the packet builder
			auto &fbb()
			{
				return _fbb;
			}

			// Get Net Stats
			auto &stats() const
			{
				return netStats;
			}

			// IDA
			ssize_t HandleEvent(hook_type_t, int, va_list);

		  signals:
			void Connected();
			void Disconnected(uint32_t);
			void Broadcasted(int);
			void StatsUpdated(const net::NetStats &);

		  private:
			// network events
			void OnConnectRequest() override;
			void OnDisconnect(uint32_t) override;
			void ProcessPacket(uint8_t *, size_t) override;

			utils::Storage _storage;
			bool _active = false;
			QScopedPointer<net::NetClient> _client;

			net::FbsBuilder _fbb;
			uint32_t _heartBeatCount = 0;

			using IdaEventType_t = std::pair<hook_type_t, int>;
			std::map<IdaEventType_t, SyncHandler *> _idaEvents;
			std::map<protocol::MsgType, SyncHandler *> _netEvents;

			QScopedPointer<QTimer> _statsTimer;
		};
	} // namespace sync
} // namespace noda