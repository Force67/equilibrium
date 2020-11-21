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

  using logcallback_t = void (*)(const char *);

  class ServerImpl;

  class Server {
  public:
	inline bool StatusOk(ServerStatus s) const
	{
	  return s == ServerStatus::Success;
	}

	Server(int16_t port = 0);
	~Server();

	ServerStatus Initialize(bool useStorage);

	void Update();

	bool IsListening() const;
	int16_t GetPort() const;

	static void SetLogCallback(logcallback_t);

  private:
	std::unique_ptr<ServerImpl> _impl;
  };
} // namespace noda