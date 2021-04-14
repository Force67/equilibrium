// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <network/tksp/tksp_host.h>
#include <network/tksp/tksp_utils.h>

#include <base/logging.h>
#include <base/object_pool.h>

namespace network::tksp {

struct Host::OutgoingCommand {
  explicit OutgoingCommand(PeerBase::Id target_id,
                           size_t size,
                           std::unique_ptr<uint8_t[]> data) {
    this->target_id = target_id;
    this->size = size;
    this->data = std::move(data);
  }

  PeerBase::Id target_id;
  uint32_t size;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<OutgoingCommand> key;
};

Host::Host(TkspDelegate& del) : delegate_(del) {
  // as defined in PeerBase we currently reserve n id's for
  // system purposes so we block 10 for future purposes
  constexpr uint32_t kReserveSize = 10u;

  seed_ = GetRandomSeed(kReserveSize);
  // pool pulls in a lot of template definitions so we forward declare it
  pool_ = std::make_unique<base::object_pool<OutgoingCommand>>();
}

void Host::ReadChunk(PeerBase& peer) {
  ssize_t count = 0;

  while ((count = peer.sock.read_n(chunk_buffer_, sizeof(chunk_buffer_))) > 0) {
    // mark the peer as still alive
    peer.Touch();

    if (count <= sizeof(Chunkheader)) {
      LOG_WARNING("Ignored invalid chunk (size of {} is too small)", count);
      continue;
    }

    if (count > Chunkheader::kMaxSize) {
      // TODO: fragmented headers... or a different allocation model
      LOG_ERROR("FIXME: chunk is too big to handle..");
      continue;
    }

    Chunkheader* header = reinterpret_cast<Chunkheader*>(chunk_buffer_);
    if (header->type >= Chunkheader::Type::kNumMessages ||
        header->type <= Chunkheader::Type::kInvalid) {
      LOG_WARNING("Ignored invalid chunk identifier: {}", header->type);
      continue;
    }

    // note that this is designed in a way where the user has to immediately
    // make a copy of chunk_buffer to avoid choking the net thread... this is
    // done for performance reasons and to avoid double queuing
    if (header->type == Chunkheader::Type::kPayload) {
      delegate_.ProcessData(peer.id, chunk_buffer_, count);
    }
  }
}

void Host::Process() {
  for (size_t i = 0; i < peer_list_.size(); i++) {
    // a bunch of logic to validate the peer list
    if (!peer_list_[i]) {
      LOG_WARNING("removing empty peer list entry (index: {})", i);
      peer_list_.erase(peer_list_.begin() + i);
      continue;
    }

    PeerBase& peer = (*peer_list_[i]);
    if (!peer.sock.is_open() || peer.IsDead()) {
      auto reason = !peer.sock.is_open() ? NotifyQuit::Reason::kConnectionLost
                                         : NotifyQuit::Reason::kTimeout;

      NotifyQuit notification{reason};
      delegate_.OnDisconnection(peer.id, notification);

      peer_list_.erase(peer_list_.begin() + i);
      continue;
    }

    // check and process any messages received by the peer
    ReadChunk(peer);
  }
}

void Host::QueueOutgoingCommand(Chunkheader::Type command_type,
                                PeerBase::Id identifier,
                                const uint8_t* data,
                                size_t size) {
  size += sizeof(Chunkheader);
  auto buffer = std::make_unique<uint8_t[]>(size);

  // allocate using placement new
  Chunkheader* header =
      new (buffer.get()) Chunkheader(command_type, static_cast<uint32_t>(size));
  std::memcpy(buffer.get() + sizeof(Chunkheader), data, size);

  OutgoingCommand* command =
      pool_->construct(identifier, size, std::move(data));
  outgoing_queue_.push(&command->key);
}

void Host::WriteChunks() {
  while (auto* entry = outgoing_queue_.pop(&OutgoingCommand::key)) {
    // broadcast it to all peers
    if (entry->target_id == PeerBase::kAllConnectId) {
      for (auto& p : peer_list_) {
        auto& peer = (*p);
        peer.sock.write_n(entry->data.get(), entry->size);
      }
      // send @ peer
    } else if (auto* peer = PeerById(entry->cid)) {
      peer->Send(entry->data.get(), entry->dataSize);
    }

    pool_->destruct(entry);
  }
}
}  // namespace network::tksp