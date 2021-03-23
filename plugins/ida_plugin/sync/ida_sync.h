// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <QTimer>
#include <unordered_map>

#include <sync/engine/sync_client.h>

#include "client_runner.h"
#include "request_runner.h"
#include "ida_event_handler.h"
#include "storage/sync_data.h"

namespace sync {
struct StaticHandler;
}

class Plugin;

class RequestContext {
 public:

};

class IdaSync final : public sync::SyncClientDelegate, public QObject {
  Q_OBJECT;
 public:

  explicit IdaSync(Plugin&);
 ~IdaSync();

 enum class State {
   kDisabled,
   kInitialized,
   kPending,
   kConfiguring,
   kActive
 };

 enum class Notification {
   kUserJoined,
   kUsersJoined,
   kUserQuit,
   kUserHint,
   kReserved
 };

private:
 void OnConnection(const sockpp::inet_address&) override;
 void OnDisconnected(int reason) override;
 void ConsumeMessage(const protocol::MessageRoot*, size_t) override;

 void HandleAuthAck(const protocol::MessageRoot*);
 void HandleUserEvent(const protocol::MessageRoot*);

 void BindStaticHandlers();
 void SetState(State);
 void SendProjectInfo();

signals:
 void StateChange(State);
 void Notify(Notification);

public:
 struct Stats {
   int idaEventNum_;
   int netEventNum_;
 } stats;
 int userCount = 1;

 using IdaEventType_t = std::pair<hook_type_t, int>;
 using IDAEvents_t = std::unordered_map<IdaEventType_t, sync::StaticHandler*>;
 using NetEvents_t = std::unordered_map<int, sync::StaticHandler*>;

 inline NetEvents_t& NetEvents() { return netEvents_; }
 inline IDAEvents_t& IdaEvents() { return idaEvents_; }

private:
 sync::SyncClient client_;
 flatbuffers::FlatBufferBuilder fbb_;
 ClientRunner netRunner_;
 RequestRunner reqRunner_;
 SyncData data_;
 QTimer timer_;
 Plugin& plugin_;

 IDAEvents_t idaEvents_;
 NetEvents_t netEvents_;
 IDAEventHandler idaHandler_;
 State state_;
 Notification notf_;

private:
 Q_ENUM(IdaSync::State)
 Q_ENUM(IdaSync::Notification)
};