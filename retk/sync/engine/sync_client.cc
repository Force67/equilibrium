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

static base::object_pool<SyncClient::Packet> s_Pool;

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

  // only store ID to keep things thread safe..
  Packet* item = s_Pool.construct();
  item->dataSize = static_cast<uint32_t>(buf.GetSize());
  item->data = std::make_unique<uint8_t[]>(buf.GetSize());
  std::memcpy(item->data.get(), buf.GetBufferPointer(), buf.GetSize());

  queue_.push(&item->key);
}

bool SyncClient::Process() {
  if (!network::TCPClient::Update())
    return false;

  while (auto* packet = queue_.pop(&Packet::key)) {
    connection_.write_n(packet->data.get(), packet->dataSize);
    s_Pool.destruct(packet);
  }

  return true;
}
}