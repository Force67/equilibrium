// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Server.h"

#include "DataHandler.h"
#include "UserRegistry.h"

#include "network/TCPServer.h"

namespace protocol {
  class Message;
}

namespace noda {

  class ServerImpl final : public network::TCPServerConsumer {
	friend class noda::Server;

  public:
	ServerImpl(int16_t port);
	~ServerImpl() = default;

	ServerStatus Initialize(bool useStorage);

	void Update();

	UserRegistry &Registry()
	{
	  return _userRegistry;
	}

  private:
	// impl for: TCPServerConsumer
	void OnDisconnection(network::connectid_t) override;
	void ConsumeMessage(network::connectid_t, const uint8_t *ptr, size_t size) override;

	void HandleAuth(network::connectid_t, const protocol::Message *message);

  private:
	bool _listening = false;
	std::string _loginToken = "";

	network::ScopedSocket _socket;
	network::TCPServer _server;
	UserRegistry _userRegistry;
	DataHandler _dataHandler;

	flatbuffers::FlatBufferBuilder _fbb;

	using timestamp_t = std::chrono::high_resolution_clock::time_point;
	timestamp_t _tickTime;
	float _freeTime = 0;
  };
} // namespace noda