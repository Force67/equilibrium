// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync_server.h"
#include <base/object_pool.h>

#include "protocol/generated/message_root_generated.h"

namespace sync {

void SyncServerDelegate::ProcessData(cid_t src,
                                     const uint8_t* data,
                                     size_t len) {
  flatbuffers::Verifier verifier(data, len);
  if (!protocol::VerifyMessageRootBuffer(verifier))
    return;

  const protocol::MessageRoot* root =
      protocol::GetMessageRoot(static_cast<const void*>(data));

  ConsumeMessage(src, root, len);
}

SyncServer::SyncServer(SyncServerDelegate& d)
    : delegate_(d), network::TCPServer(d) {}

SyncServer::~SyncServer() {}

void SyncServer::Broadcast(protocol::MsgType type,
                           FbsBuffer& buf,
                           FbsRef<void> ref) {
  const auto packet = protocol::CreateMessageRoot(buf, type, ref);
  buf.Finish(packet);

  TCPServer::Broadcast(network::OpCode::kData, buf.GetBufferPointer(),
                       buf.GetSize());
}

void SyncServer::Broadcast(const protocol::MessageRoot* root, size_t len) {
  TCPServer::Broadcast(network::OpCode::kData,
                       static_cast<const uint8_t*>(root->msg()), len);
}

void SyncServer::Send(cid_t cid,
                      protocol::MsgType type,
                      FbsBuffer& buf,
                      FbsRef<void> ref) {
  const auto packet = protocol::CreateMessageRoot(buf, type, ref);
  buf.Finish(packet);

  TCPServer::Send(network::OpCode::kData, cid, buf.GetBufferPointer(),
                  buf.GetSize());
}

void SyncServer::Process() {
  network::TCPServer::Tick();
}
}