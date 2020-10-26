// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Server.h"
#include "NdUser.h"

#include "DataHandler.h"

#include "netlib/Server.h"

namespace noda {

  class ServerImpl final : public netlib::Server {
	friend class noda::Server;

  public:
	ServerImpl(uint16_t port);
	~ServerImpl() = default;

	ServerStatus Initialize(bool useStorage);

	void Update();

	// thread safe due to the order of calls
	// in the net thread
	userptr_t UserById(netlib::connectid_t cid);

	void CreatePacket(netlib::connectid_t cid,
	                  protocol::MsgType type,
	                  FbsBuffer &buffer,
	                  flatbuffers::Offset<void> packet);

  private:
	void OnDisconnection(netlib::Peer *peer) override;
	void OnConsume(netlib::Peer *peer, netlib::Packet *packet) override;

	void HandleAuth(netlib::Peer *source, const protocol::Message *message);

  private:
	bool _listening = false;
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