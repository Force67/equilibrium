// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "TCPPeer.h"
#include "flatbuffers/flatbuffers.h"

#include <utility/ObjectPool.h>
#include <utility/DetachedQueue.h>

namespace network {

  class TCPServerConsumer {
  public:
	virtual ~TCPServerConsumer() = default;

	virtual void OnConnection(const TCPPeer &) = 0;
	virtual void OnDisconnection(const TCPPeer &) = 0;

	virtual void ConsumeMessage(const uint8_t *, size_t size) = 0;
  };

  class TCPServer {
  public:
	explicit TCPServer(TCPServerConsumer &consumer);

	bool Host(int16_t port);

	void Tick();

	bool Drop(connectionid_t);

	void SendPacket(connectionid_t cid,
	                protocol::MsgType type,
	                FbsBuffer &buffer,
	                FbsRef<void> packet);

	TCPPeer *PeerById(connectionid_t);

  private:
	// named after martin who discovered
	// a true source of randomness
	uint32_t GetMartinsRandomSeed();

	sockpp::tcp_acceptor _acc;
	sockpp::inet_address _addr;

	std::vector<TCPPeer> _peers;
	TCPServerConsumer &_consumer;

	uint8_t _workbuf[1024]{};
	uint32_t _seed;

	struct Packet {
	  FbsBuffer buffer;
	  connectionid_t cid;
	  utility::detached_queue_key<Packet> key;
	};

	utility::object_pool<Packet> _packetPool;
	utility::detached_mpsc_queue<Packet> _queue;
  };
} // namespace network