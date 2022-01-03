// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

struct MsgContext;
class Plugin;

class IdaSync {
 public:
  IdaSync(Plugin&){};
};

#if 0
#include <QTimer>
#include <unordered_map>

#include <sync/engine/sync_client.h>
#include <network/base/context_holder.h>

#include "client_runner.h"
#include "request_runner.h"
#include "ida_event_handler.h"
#include "storage/sync_data.h"

namespace sync {
struct StaticHandler;
}

class Plugin;
class IdaSync;

struct MsgContext {
  inline explicit MsgContext(IdaSync& s) : sync(s) {}

  IdaSync& sync;
  sync::FbsBuffer idbBuf;
  sync::FbsBuffer idpBuf;

  template<typename... Args>
  bool SendIdb(Args&&... args) {
    sync.Client().Send(idbBuf, args...);
    return true;
  }

  template <typename... Args>
  bool SendIdp(Args&&... args) {
    sync.Client().Send(idpBuf, args...);
    return true;
  }
};

class IdaSync final : public QObject, public network::ClientDelegate {
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

 bool Connected() const { return client_.Connected(); }
 bool IsOnline() const { return state_ == State::kActive; }
 void Stop() { netRunner_.Stop(); }
 bool Start() { return netRunner_.Start(); }

private:
 void OnConnection(const sockpp::inet_address&) override;
 void OnDisconnected(network::QuitReason) override;
 void ProcessData(const uint8_t*, size_t) override;

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
 // needs to be fixed eventually back to unoredered map
 using IDAEvents_t = std::map<IdaEventType_t, sync::StaticHandler*>;
 using NetEvents_t = std::unordered_map<int, sync::StaticHandler*>;

 inline NetEvents_t& NetEvents() { return netEvents_; }
 inline IDAEvents_t& IdaEvents() { return idaEvents_; }
 inline auto& Client() { return client_; }
 inline MsgContext& Context() { return context_; }
 inline auto& Data() { return data_; }
private:
 network::ContextHolder netContext_;
 sync::SyncClient client_;
 sync::FbsBuffer fbb_;
 MsgContext context_;
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
#endif