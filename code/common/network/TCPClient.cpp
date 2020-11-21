// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <utility/ObjectPool.h>
#include "TCPClient.h"

namespace network {
  inline utility::object_pool<OutPacket> s_packetPool;

  TCPClient::TCPClient(TCPClientConsumer &consumer) :
      _consumer(consumer)
  {
  }

  bool TCPClient::Connect(const char *addr, int16_t port)
  {
	if(!addr)
	  addr = "localhost";

	if(!_conn.connect(sockpp::inet_address(addr, port)))
	  return false;

	if(!_conn.set_non_blocking())
	  return false;

	//_conn.read_timeout();

	_conn.set_option(SOL_SOCKET, SO_KEEPALIVE, 1);
	//_conn.set_option(IPPROTO_TCP, )

	// TODO: think about timeout..
	//_conn.read_timeout();

	return true;
  }

  std::string TCPClient::LastError() const
  {
	return _conn.last_error_str();
  }

  void TCPClient::Disconnect()
  {
	_conn.reset();
  }

  void TCPClient::SendPacket(pt::MsgType type, FbsBuffer &buf, FbsRef<void> ref)
  {
	const auto packet = protocol::CreateMessage(buf, type, ref);
	buf.Finish(packet);

	OutPacket *item = s_packetPool.construct();
	item->buffer = std::move(buf);

	_outQueue.push(&item->key);
  }

  void TCPClient::Tick()
  {
	if(!_conn.is_connected())
	  return;

	ssize_t n = 0;
	while((n = _conn.read(buf, sizeof(buf))) > 0) {
	  _consumer.ConsumeMessage(buf, n);
	}

	while(auto *packet = _outQueue.pop(&OutPacket::key)) {
	  _conn.write_n(
	      packet->buffer.GetBufferPointer(),
	      packet->buffer.GetSize());

	  s_packetPool.destruct(packet);
	}
  }
} // namespace network