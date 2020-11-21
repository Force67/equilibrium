// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>

namespace noda {

  enum class ServerStatus {
	Success,
	NetError,
	FsError,
  };

  class ServerImpl;

  class Server {
  public:
	inline bool StatusOk(ServerStatus s) const
	{
	  return s == ServerStatus::Success;
	}

	Server(int16_t port);
	~Server();

	ServerStatus Initialize(bool useStorage);

	void Update();

	bool IsListening() const;
	int16_t GetPort() const;

  private:
	std::unique_ptr<ServerImpl> _impl;
  };
} // namespace noda