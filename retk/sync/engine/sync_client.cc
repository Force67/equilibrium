// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync_client.h"
#include <base/object_pool.h>

#include "protocol/generated/message_root_generated.h"

namespace sync {

SyncClient::SyncClient(network::ClientDelegate& d) : network::TCPClient(d) {}

void SyncClient::Send(FbsBuffer& buffer,
                      protocol::MsgType type,
                      FbsRef<void> ref) {
  const auto packet = protocol::CreateMessageRoot(buffer, type, ref);
  buffer.Finish(packet);

  QueueCommand(network::CommandId::kData, buffer.GetBufferPointer(),
               buffer.GetSize());
}

}  // namespace sync