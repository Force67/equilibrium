// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync_server.h"
#include <base/object_pool.h>

#include "protocol/generated/message_root_generated.h"

namespace sync {

constexpr cid_t kAtAllId = std::numeric_limits<uint32_t>::max() - 1;

struct SyncServer::Packet {
  cid_t cid;
  uint32_t dataSize;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Packet> key;
};

static base::object_pool<SyncServer::Packet> s_Pool;

void SyncServerDelegate::ProcessData(cid_t src, const uint8_t* data, size_t len) {

  flatbuffers::Verifier verifier(data, len);
  if (!protocol::VerifyMessageRootBuffer(verifier))
    return;

  const protocol::MessageRoot* root =
      protocol::GetMessageRoot(static_cast<const void*>(data));

  ConsumeMessage(src, root, len);
}

SyncServer::SyncServer(SyncServerDelegate& d) : 
    delegate_(d), network::TCPServer(d) {
}

SyncServer::~SyncServer() {

}

void SyncServer::Broadcast(protocol::MsgType t, FbsBuffer& buf, FbsRef<void> ref) {
  Send(kAtAllId, t, buf, ref);
}

void SyncServer::Broadcast(const protocol::MessageRoot* root, size_t len) {
  Packet* item = s_Pool.construct();
  item->cid = kAtAllId;
  item->dataSize = static_cast<uint32_t>(len);
  item->data = std::make_unique<uint8_t[]>(len);
  std::memcpy(item->data.get(), root->msg(), len);
}

void SyncServer::Send(cid_t cid, protocol::MsgType type, FbsBuffer& buf, FbsRef<void> ref) {
  const auto packet = protocol::CreateMessageRoot(buf, type, ref);
  buf.Finish(packet);

  // only store ID to keep things thread safe..
  Packet* item = s_Pool.construct();
  item->cid = cid;
  item->dataSize = static_cast<uint32_t>(buf.GetSize());
  item->data = std::make_unique<uint8_t[]>(buf.GetSize());
  std::memcpy(item->data.get(), buf.GetBufferPointer(), buf.GetSize());

  queue_.push(&item->key);
}

void SyncServer::Process() {
  network::TCPServer::Tick();

    // work out queue
  while (auto* packet = queue_.pop(&Packet::key)) {
    if (packet->cid == kAtAllId) {
      for (auto& p : _peers)
        p.sock.write_n(static_cast<const void*>(packet->data.get()),
                       static_cast<size_t>(packet->dataSize));
    }
    else if (auto* peer = PeerById(packet->cid)) {
      peer->sock.write_n(
          static_cast<const void*>(packet->data.get()),
          static_cast<size_t>(packet->dataSize));
    }

    s_Pool.destruct(packet);
  }
}
}