// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync_client.h"
#include <base/object_pool.h>

#include "protocol/generated/message_root_generated.h"

namespace sync {

struct SyncClient::Packet {
  uint32_t dataSize;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Packet> key;
};

void SyncClientDelegate::ProcessData(const uint8_t* data, size_t len) {
  flatbuffers::Verifier verifier(data, len);
  if (!protocol::VerifyMessageRootBuffer(verifier))
    return;

  const protocol::MessageRoot* root =
      protocol::GetMessageRoot(static_cast<const void*>(data));

  ConsumeMessage(root, len);
}

SyncClient::SyncClient(SyncClientDelegate &d) : 
    delegate_(d), network::TCPClient(d) {}

void SyncClient::Send(FbsBuffer& buf, protocol::MsgType type, FbsRef<void> ref) {
  const auto packet = protocol::CreateMessageRoot(buf, type, ref);
  buf.Finish(packet);
  
  TCPClient::Send(network::OpCode::kData, buf.GetBufferPointer(), buf.GetSize());
}

bool SyncClient::Process() {
  if (!network::TCPClient::Update())
    return false;



  return true;
}
}