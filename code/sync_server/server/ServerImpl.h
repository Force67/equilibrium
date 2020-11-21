// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Server.h"
#include "NdUser.h"

#include "DataHandler.h"

#include "network/TCPServer.h"

namespace noda {

  class ServerImpl final : public network::TCPServerConsumer {
	friend class noda::Server;

  public:
	ServerImpl(int16_t port);
	~ServerImpl() = default;

	ServerStatus Initialize(bool useStorage);

	void Update();

	// thread safe due to the order of calls
	// in the net thread
	userptr_t UserById(netlib::connectid_t cid);

  private:
	void OnConnection(const network::TCPPeer& peer) override;
	void OnDisconnection(const network::TCPPeer& peer) override;
	void ConsumeMessage(network::TCPPeer& source, const uint8_t* ptr, size_t size) override;

	void HandleAuth(netlib::Peer *source, const protocol::Message *message);

  private:

	bool _listening = false;
	network::TCPServer _server;

	std::string _token = "";
	DataHandler _datahandler;

	flatbuffers::FlatBufferBuilder _fbb;

	using timestamp_t = std::chrono::high_resolution_clock::time_point;
	timestamp_t _tickTime;
	float _freeTime = 0;

	std::vector<userptr_t> _userRegistry;
	utility::detached_mpsc_queue<OutPacket> _packetQueue;
  };
} // namespace noda