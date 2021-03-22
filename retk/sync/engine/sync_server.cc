// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync_server.h"
#include <base/object_pool.h>

#include "protocol/generated/message_root_generated.h"

namespace sync {

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

  ConsumeMessage(src, root);
}

SyncServer::SyncServer(SyncServerDelegate& d) : 
    delegate_(d), network::TCPServer(d) {
}

SyncServer::~SyncServer() {

}

void SyncServer::QueuePacket(network::connectid_t cid, flatbuffers::FlatBufferBuilder& fbb) {
  // only store ID to keep things thread safe..
  Packet* item = s_Pool.construct();
  item->cid = cid;
  item->dataSize = static_cast<uint32_t>(fbb.GetSize());
  item->data = std::make_unique<uint8_t[]>(fbb.GetSize());
  std::memcpy(item->data.get(), fbb.GetBufferPointer(), fbb.GetSize());

  queue_.push(&item->key);
}

void SyncServer::Process() {
  network::TCPServer::Tick();

    // work out queue
  while (auto* packet = queue_.pop(&Packet::key)) {
    if (const auto* peer = PeerById(packet->cid)) {
      peer->sock.write_n(packet->data.get(),
                         packet->dataSize);
    }

    s_Pool.destruct(packet);
  }
}
}