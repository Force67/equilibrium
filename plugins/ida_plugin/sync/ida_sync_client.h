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

class IDASyncClient final : public sync::SyncClientDelegate {
public:
  IDASyncClient();
 ~IDASyncClient();

private:
 void OnConnection(const sockpp::inet_address&) override;
 void OnDisconnected(int reason) override;
 void ConsumeMessage(const protocol::MessageRoot*) override;

 void HandleAuthAck(const protocol::MessageRoot*);
 void HandleUserEvent(const protocol::MessageRoot*);

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
 ClientRunner netRunner_;
 RequestRunner reqRunner_;
 SyncData data_;
 QTimer timer_;

 IDAEvents_t idaEvents_;
 NetEvents_t netEvents_;
 IDAEventHandler idaHandler_;
};